
#include "metadata.hpp"
#include "bonk/middleend/annotators/basic_symbol_annotator.hpp"
#include "bonk/middleend/annotators/type_annotator.hpp"
#include "bonk/middleend/annotators/type_visitor.hpp"

std::filesystem::path bonk::SourceMetadata::get_meta_path(const std::filesystem::path& path) {
    return path.parent_path() / ".bscache" / (path.stem().string() + ".meta");
}

// These structures are moved to .cpp file because they are not used outside of this file

class MetadataASTBuilderVisitor : public bonk::ASTCloneVisitor {
    bonk::MiddleEnd& middle_end;

  public:
    MetadataASTBuilderVisitor(bonk::MiddleEnd& middle_end) : middle_end(middle_end) {
        // Source position should be copied to the meta AST,
        // because it is used to resolve external symbols implicit imports
        copy_source_positions = true;
    }

    void visit(bonk::TreeNodeBlockDefinition* node) override;
    void visit(bonk::TreeNodeVariableDefinition* node) override;
};

class ASTCloneWithExternalSymbolsVisitor : public bonk::ASTCloneVisitor {
    bonk::MiddleEnd& source_middle_end;
    bonk::MiddleEnd& target_middle_end;

  public:
    ASTCloneWithExternalSymbolsVisitor(bonk::MiddleEnd& source_middle_end,
                                       bonk::MiddleEnd& target_middle_end)
        : source_middle_end(source_middle_end), target_middle_end(target_middle_end) {
        // Source position should be copied to the meta AST,
        // because it is used to resolve external symbols implicit imports
        copy_source_positions = true;
    }

    void visit(bonk::TreeNodeIdentifier* node) override;
};

// Moves all the strings from the buffer of the original AST to the buffer of the new AST,
// so it becomes independent of the original AST
class MetadataASTStringMoveVisitor : public bonk::ASTFieldWalker<MetadataASTStringMoveVisitor> {
    bonk::AST& ast;

  public:
    MetadataASTStringMoveVisitor(bonk::AST& ast) : ast(ast), ASTFieldWalker(*this) {
    }

    void operator()(bonk::ParserPosition& field, std::string_view) {
        field.filename = ast.buffer.get_symbol(field.filename);
    }

    void operator()(std::string_view& field, std::string_view) {
        field = ast.buffer.get_symbol(field);
    }

    template <typename T> void operator()(std::unique_ptr<T>& field, std::string_view) {
        if (field)
            field->accept(this);
    }

    template <typename T> void operator()(std::list<T>& field, std::string_view) {
        for (auto& node : field) {
            if (node)
                node->accept(this);
        }
    }

    template <typename T> void operator()(T& field, std::string_view) {
    }

    void move_strings() {
        ast.root->accept(this);
    }
};

class TypeReferenceBuilderVisitor : public bonk::ASTVisitor {
  public:
    TypeReferenceBuilderVisitor(bonk::MiddleEnd& middle_end, bonk::TypeReferenceMetadata& metadata)
        : middle_end(middle_end), metadata(metadata) {

        // Reserve 0 for the empty string
        get_id_for_filename("");
    }

    void visit(bonk::TreeNodeIdentifier* node) override;

  private:
    bonk::TypeReferenceMetadata& metadata;
    bonk::MiddleEnd& middle_end;
    std::unordered_map<std::string_view, unsigned long> registered_file_names;

    long get_id_for_filename(std::string_view filename);
};

class ExternalTableFillerVisitor : public bonk::ASTVisitor {
  public:
    ExternalTableFillerVisitor(bonk::MiddleEnd& middle_end, bonk::TypeReferenceMetadata& metadata)
        : middle_end(middle_end), metadata(metadata) {
    }

    void visit(bonk::TreeNodeIdentifier* node) override;

  private:
    bonk::TypeReferenceMetadata& metadata;
    bonk::MiddleEnd& middle_end;
    int current_node_id = 0;
};

void ASTCloneWithExternalSymbolsVisitor::visit(bonk::TreeNodeIdentifier* node) {
    ASTCloneVisitor::visit(node);

    // If the identifier is mentioned in external_symbol_table,
    // this information should be moved to the meta AST as well

    auto it = source_middle_end.external_symbol_table.external_symbol_def_files.find(node);
    if (it == source_middle_end.external_symbol_table.external_symbol_def_files.end()) {
        return;
    }

    auto& file_index = it->second;
    auto file_name = source_middle_end.external_symbol_table.get_external_file(file_index);

    target_middle_end.external_symbol_table.register_symbol((bonk::TreeNodeIdentifier*)result.get(),
                                                            file_name);
}

void MetadataASTBuilderVisitor::visit(bonk::TreeNodeBlockDefinition* node) {
    // Block bodies are stripped from the meta AST, so in order for compiler
    // to determine the return type of the block, it should be stored explicitly
    // in the meta AST

    ASTCloneVisitor::visit(node);
    auto copy = (bonk::TreeNodeBlockDefinition*)result.get();

    copy->block_name = clone(node->block_name.get());
    copy->block_parameters = clone(node->block_parameters.get());
    copy->return_type = clone(node->return_type.get());

    if (!copy->return_type) {
        // Find the return type in the type table
        auto type = middle_end.type_table.get_type(node);
        assert(type->kind == bonk::TypeKind::blok);

        auto block_type = (bonk::BlokType*)type;
        copy->return_type = bonk::TypeToASTConvertVisitor().convert(block_type->return_type.get());
    }
}

void MetadataASTBuilderVisitor::visit(bonk::TreeNodeVariableDefinition* node) {

    ASTCloneVisitor::visit(node);
    auto copy = (bonk::TreeNodeVariableDefinition*)result.get();

    copy->variable_name = clone(node->variable_name.get());
    copy->variable_type = clone(node->variable_type.get());
    copy->variable_value = clone(node->variable_value.get());
}

void ExternalTableFillerVisitor::visit(bonk::TreeNodeIdentifier* node) {
    unsigned long file_index = metadata.identifier_files[current_node_id];
    std::string_view file_name = metadata.file_names[file_index];
    middle_end.external_symbol_table.register_symbol(node, file_name);
    current_node_id++;
}

bonk::SourceMetadata::SourceMetadata(bonk::MiddleEnd& middle_end,
                                     const std::filesystem::path& source_path)
    : source_path(source_path), middle_end(middle_end) {
    meta_path = get_meta_path(source_path);
    read_metadata();
}

bool bonk::SourceMetadata::is_up_to_date_for(const bonk::SourceMetadata& other) {
    if (!meta_ast.root)
        return false;
    return check_date > other.changed_date;
}

void bonk::SourceMetadata::rebuild_metadata_ast(bonk::TreeNodeProgram* ast) {
    metadata_rebuilt = true;

    // Create header AST and move all the identifier strings to its
    // buffer, so it becomes independent of the original AST
    meta_ast.root = MetadataASTBuilderVisitor(middle_end).clone(ast);
    MetadataASTStringMoveVisitor(meta_ast).move_strings();

    // Perform symbol/type annotation on the meta AST,
    // so added explicit return types / variable types / etc. are annotated

    // Create a new MiddleEnd instance for the meta AST
    MiddleEnd meta_middle_end(middle_end.linked_compiler);

    // Modules should be cloned along with their external symbol table
    // TODO: maybe not to clone modules at all?
    auto cloner = ASTCloneWithExternalSymbolsVisitor(middle_end, meta_middle_end);
    cloner.copy_source_positions = true;

    for (auto& [path, module] : middle_end.external_modules) {
        if (meta_middle_end.has_module(path))
            continue;

        AST copied_module;
        copied_module.root = cloner.clone(module->module_ast.root.get());
        MetadataASTStringMoveVisitor(copied_module).move_strings();

        meta_middle_end.add_external_module(path, std::move(copied_module));
    }

    meta_middle_end.annotate_ast(meta_ast, nullptr);

    TypeReferenceBuilderVisitor type_reference_builder(meta_middle_end, type_reference_metadata);
    meta_ast.root->accept(&type_reference_builder);

    write_metadata_if_needed();
}

bool bonk::SourceMetadata::metadata_is_newer_than_source() {
    if (!meta_ast.root)
        return false;
    return check_date > std::filesystem::last_write_time(source_path);
}

bonk::TreeNode* bonk::SourceMetadata::get_meta_ast() {
    return meta_ast.root.get();
}

bonk::AST bonk::SourceMetadata::to_ast() && {
    meta_file_contents = {};
    return std::move(meta_ast);
}

void bonk::SourceMetadata::read_metadata() {
    // Read the meta file to meta_file_contents
    std::ifstream meta_file{meta_path.string()};

    if (!meta_file.is_open()) {
        return;
    }

    check_date = std::filesystem::last_write_time(meta_path);

    meta_file.seekg(0, std::ios::end);
    size_t file_size = meta_file.tellg();
    meta_file.seekg(0, std::ios::beg);
    char* buffer = meta_ast.buffer.reserve_data(file_size + 1);
    meta_file_contents = {buffer, (size_t)file_size};
    meta_file.read(buffer, meta_file_contents.size());

    metadata_rebuilt = false;

    bonk::BufferInputStream input{
        std::string_view(meta_file_contents.data(), meta_file_contents.size())};

    // Read the changed_date from the beginning of the meta file
    input.get_stream().read((char*)&changed_date, sizeof(changed_date));

    // Read the AST from the meta file
    bonk::BinaryASTDeserializer ast_deserializer{input};
    auto read_ast = ast_deserializer.read();

    assert(read_ast->type == TreeNodeType::n_program);
    meta_ast.root = std::unique_ptr<TreeNodeProgram>((TreeNodeProgram*)(read_ast.release()));

    // Read the type_reference_metadata from the meta file
    type_reference_metadata.decode(input);
}

void bonk::SourceMetadata::write_metadata_if_needed() {
    if (!metadata_rebuilt)
        return;

    std::stringstream output_stringstream;
    bonk::StdOutputStream output_stream{output_stringstream};

    // Write the changed_date to the beginning of the meta file
    output_stream.get_stream().write((char*)&changed_date, sizeof(changed_date));

    // Write the AST to the meta file
    bonk::BinaryASTSerializer ast_serializer{output_stream};
    meta_ast.root->accept(&ast_serializer);

    // Write the type_reference_metadata to the meta file
    type_reference_metadata.encode(output_stream);

    std::string new_metadata_contents = output_stringstream.str();

    // If the contents are not the same, update the change date
    if (new_metadata_contents != meta_file_contents) {
        changed_date = std::filesystem::file_time_type::clock::now();
        // Update the check_date in the meta_file_contents as well
        std::memcpy(new_metadata_contents.data(), &changed_date, sizeof(changed_date));
    }

    metadata_rebuilt = false;

    // Create intermediate directories if needed
    std::filesystem::create_directories(meta_path.parent_path());

    // Write the new contents to the meta file
    FileOutputStream file_output_stream{meta_path.string()};
    file_output_stream.get_stream().write(new_metadata_contents.data(),
                                          new_metadata_contents.size());
}

void bonk::TypeReferenceMetadata::encode(const bonk::OutputStream& stream) {
    unsigned long size = file_names.size();
    stream.get_stream().write((char*)&size, sizeof(unsigned long));

    for (int i = 0; i < size; i++) {
        auto& filename = file_names[i];
        unsigned long length = filename.length();
        stream.get_stream().write(filename.data(), length);
        stream.get_stream().write("\0", 1);
    }

    size = identifier_files.size();
    stream.get_stream().write((char*)&size, sizeof(unsigned long));

    for (int i = 0; i < size; i++) {
        unsigned long id = identifier_files[i];
        stream.get_stream().write((char*)&id, sizeof(unsigned long));
    }
}

void bonk::TypeReferenceMetadata::decode(const bonk::BufferInputStream& stream) {
    unsigned long size = 0;
    stream.get_stream().read((char*)&size, sizeof(unsigned long));

    file_names.clear();
    for (int i = 0; i < size; i++) {
        const char* start = stream.input.data() + stream.tell();
        do {
            int c = stream.get_stream().get();
            if (c == '\0' || c == EOF)
                break;
        } while (true);
        const char* end = stream.input.data() + stream.tell() - 1;

        file_names.emplace_back(start, end - start);
    }

    size = 0;
    stream.get_stream().read((char*)&size, sizeof(unsigned long));

    identifier_files.clear();
    for (int i = 0; i < size; i++) {
        unsigned long id = 0;
        stream.get_stream().read((char*)&id, sizeof(unsigned long));
        identifier_files.push_back(id);
    }
}

long TypeReferenceBuilderVisitor::get_id_for_filename(std::string_view filename) {
    auto it = registered_file_names.find(filename);
    if (it == registered_file_names.end()) {
        auto id = registered_file_names.size();
        registered_file_names[filename] = id;
        metadata.file_names.emplace_back(filename);
        return id;
    } else {
        return it->second;
    }
}

void TypeReferenceBuilderVisitor::visit(bonk::TreeNodeIdentifier* node) {
    auto definition = middle_end.symbol_table.get_definition(node);

    if (!definition) {
        metadata.identifier_files.push_back(0);
        return;
    }

    std::string_view filename;

    if (definition.is_local()) {
        filename = definition.get_local().definition->source_position.filename;
    } else if (definition.is_external()) {
        filename = definition.get_external().file;
    }

    if (filename.empty()) {
        metadata.identifier_files.push_back(0);
        return;
    }

    auto id = get_id_for_filename(filename);
    metadata.identifier_files.push_back(id);
}

void bonk::SourceMetadata::fill_external_symbol_table(bonk::MiddleEnd& middle_end) {
    ExternalTableFillerVisitor visitor{middle_end, type_reference_metadata};
    meta_ast.root->accept(&visitor);
}