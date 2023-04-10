
#include "help_resolver.hpp"

std::filesystem::path bonk::HelpResolver::get_output_path(const std::filesystem::path& path) {
    return path.parent_path() / ".bscache" / (path.stem().string() + ".out");
}

bonk::HelpResolver::HelpResolver(bonk::Compiler& compiler) : compiler(compiler) {
}

std::optional<std::string_view> bonk::HelpResolver::get_source(bonk::Buffer& buffer,
                                                               const std::filesystem::path& path) {
    bonk::FileInputStream input_file(path.string());
    if (!input_file.get_stream()) {
        return std::nullopt;
    }

    input_file.get_stream().seekg(0, std::ios::end);
    size_t file_size = input_file.get_stream().tellg();
    input_file.get_stream().seekg(0, std::ios::beg);

    char* result = buffer.reserve_data(file_size + 1);

    input_file.get_stream().read(result, file_size);

    return std::string_view {result, file_size};
}

std::unique_ptr<bonk::SourceMetadata>
bonk::HelpResolver::get_recent_metadata_for_source(const std::filesystem::path& path) {
    assert(path.is_absolute());

    auto output_path = HelpResolver::get_output_path(path);
    compiler.report_project_file(output_path.string());

    MiddleEnd nested_middle_end(compiler);
    nested_middle_end.module_path = path;

    bool all_dependencies_are_good = true;

    auto metadata = std::make_unique<SourceMetadata>(nested_middle_end, path);

    bool should_update_metadata = !std::filesystem::exists(output_path);

    if (!should_update_metadata && metadata->metadata_is_newer_than_source()) {
        auto meta_ast = metadata->get_meta_ast();

        // Iterate over all help statements in the meta AST
        assert(meta_ast->type == TreeNodeType::n_program);

        auto meta_ast_program = (TreeNodeProgram*)meta_ast;

        for (auto& statement : meta_ast_program->help_statements) {

            auto help_string = statement->string->string_value;
            auto dependency_path = canonical(path.parent_path() /= help_string);

            if (!std::filesystem::exists(dependency_path)) {
                file_not_found(statement.get(), help_string);
                all_dependencies_are_good = false;
                continue;
            }

            auto dependency_metadata = get_recent_metadata_for_source(dependency_path);

            if(!dependency_metadata) {
                all_dependencies_are_good = false;
                continue;
            }

            if (!metadata->is_up_to_date_for(*dependency_metadata)) {
                should_update_metadata = true;
            }
        }
    } else {
        should_update_metadata = true;
    }

    if (!should_update_metadata) {
        return metadata;
    }

    // Need a nested resolver here, because otherwise
    // this->sources and this->filenames will be populated
    // with all the sources of the project tree. These fields
    // are only supposed to contain the sources of the current
    // file and meta files of its dependencies, separate resolver
    // is required for each compiled file.
    HelpResolver nested_resolver(compiler);

    auto ast = nested_resolver.get_transformed_ast(nested_middle_end, path);
    if (!ast) {
        return nullptr;
    }

    // Could have checked it earlier, but it's better to also
    // check some errors of this file as well, because otherwise
    // an error in a dependency file will lead to ignoring all
    // errors in dependent files
    if(!all_dependencies_are_good) {
        return nullptr;
    }

    if(!metadata->rebuild_metadata_ast(ast->root.get())) {
        return nullptr;
    }

    return metadata;
}

std::optional<bonk::AST> bonk::HelpResolver::get_ast(const std::filesystem::path& file_path) {
    bonk::Buffer buffer;

    auto source = get_source(buffer, file_path);
    if (!source) {
        return std::nullopt;
    }

    bonk::FileInputStream input_file(file_path.string());

    std::string_view filename_view = buffer.get_symbol(file_path.string());

    auto lexemes = bonk::Lexer(compiler).parse_file(filename_view, source.value());

    if (lexemes.empty()) {
        return std::nullopt;
    }

    auto ast_root = bonk::Parser(compiler).parse_file(&lexemes);

    if(!ast_root) {
        return std::nullopt;
    }

    return AST{.root = std::move(ast_root), .buffer = std::move(buffer)};
}

std::optional<bonk::AST> bonk::HelpResolver::get_transformed_ast(bonk::MiddleEnd& middle_end,
                                                  const std::filesystem::path& file_path) {
    middle_end.module_path = file_path;

    auto ast = get_ast(file_path);
    if (!ast) {
        return std::nullopt;
    }

    for (auto& help_statement : ast->root->help_statements) {
        auto path = file_path.parent_path() /= help_statement->string->string_value;
        auto absolute_path = std::filesystem::absolute(path);

        if (!std::filesystem::exists(absolute_path)) {
            file_not_found(help_statement.get(), help_statement->string->string_value);
            continue;
        }

        auto metafile = get_recent_metadata_for_source(absolute_path);

        if (!metafile) {
            continue;
        }

        metafile->fill_external_symbol_table(middle_end);
        middle_end.add_external_module(absolute_path, std::move(*metafile).to_ast());
    }

    if (!middle_end.transform_ast(ast.value())) {
        return std::nullopt;
    }

    auto absolute_file_path = std::filesystem::absolute(file_path);

    recompile_file(middle_end, absolute_file_path, ast->root.get());

    return ast;
}

bool bonk::HelpResolver::compile_file(const std::filesystem::path& file_path) {
    auto absolute_file_path = std::filesystem::absolute(file_path);
    return get_recent_metadata_for_source(absolute_file_path) != nullptr;
}

void bonk::HelpResolver::recompile_file(bonk::MiddleEnd& middle_end,
                                        const std::filesystem::path& path,
                                        bonk::TreeNodeProgram* ast) {
    auto hir = middle_end.generate_hir(ast);
    if (hir) {
        std::filesystem::path output_path = get_output_path(path);
        std::filesystem::create_directories(output_path.parent_path());
        bonk::FileOutputStream output_file(output_path.string());
        compiler.backend->compile_program(*hir, output_file);
    }
}

void bonk::HelpResolver::file_not_found(bonk::TreeNode* node, const std::filesystem::path& path) {
    compiler.error().at(node->source_position) << "File " << path << " doesn't exist";
}
