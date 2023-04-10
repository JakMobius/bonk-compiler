
#include "binary_ast_deserializer.hpp"

void bonk::BinaryImportMainStageCallback::operator()(bonk::OperatorType& value, std::string_view) {
    context.stream.get_stream().read((char*)&value, sizeof(value));
}

void bonk::BinaryImportMainStageCallback::operator()(bonk::NumberConstantContents& value,
                                                     std::string_view) {
    context.stream.get_stream().read((char*)&value, sizeof(value));
}

void bonk::BinaryImportMainStageCallback::operator()(bonk::TrivialTypeKind& value, std::string_view) {
    context.stream.get_stream().read((char*)&value, sizeof(value));
}

void bonk::BinaryImportMainStageCallback::operator()(bonk::ParserPosition& value,
                                                     std::string_view) {
    value.filename = context.read_string();
    context.stream.get_stream().read((char*)&value.line, sizeof(value.line));
    context.stream.get_stream().read((char*)&value.ch, sizeof(value.ch));
    context.stream.get_stream().read((char*)&value.index, sizeof(value.index));
}

void bonk::BinaryImportMainStageCallback::operator()(std::string_view& value, std::string_view) {
    value = context.read_string();
}

void bonk::BinaryImportMainStageCallback::operator()(std::string& value, std::string_view) {
    value = context.read_string();
}

void bonk::BinaryImportMainStageCallback::operator()(bonk::TreeNodeType& value, std::string_view) {
    // Do nothing, as tree node type has already been read by the read_node function
    // (it's guaranteed to be the first field in the node)
}
std::unique_ptr<bonk::TreeNode> bonk::BinaryImportMainStageCallback::read_node() {
    TreeNodeType type;

    context.stream.get_stream().read((char*)&type, sizeof(TreeNodeType));

    auto result = TreeNode::create(type);
    result->accept(context.visitor);

    return result;
}
std::unique_ptr<bonk::TreeNode> bonk::BinaryASTDeserializer::read() {

    BinaryImportContext import_context(stream);
    BinaryImportMainStageCallback field_callback(import_context);

    // Read the header length and skip it over
    unsigned int header_length = 0;
    import_context.stream.get_stream().read((char*)&header_length, sizeof(header_length));
    import_context.stream.get_stream().ignore(header_length - sizeof(header_length));

    bonk::ASTFieldWalker visitor(field_callback);

    import_context.visitor = &visitor;
    return field_callback.read_node();
}

std::string_view bonk::BinaryImportContext::read_string() {
    unsigned char is_small = 0;
    stream.get_stream().read((char*)&is_small, sizeof(unsigned char));

    const char* start = nullptr;
    const char* end = nullptr;

    if(is_small) {
        start = stream.input.begin() + stream.tell();
        end = (char*)memchr(start, 0, stream.input.size() - stream.tell());
        assert(end != nullptr);
        stream.get_stream().ignore((int)(end - start) + 1);
    } else {
        unsigned long position = 0;
        stream.get_stream().read((char*)&position, sizeof(unsigned long));
        start = string_table + position;
        end = (char*)memchr(start, 0, stream.input.size() - (start - stream.input.data()));
        assert(end != nullptr);
    }

    return {start, (size_t)(end - start)};
}
