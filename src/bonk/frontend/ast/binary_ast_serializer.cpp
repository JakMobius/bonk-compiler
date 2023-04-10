
#include <cassert>
#include "binary_ast_serializer.hpp"

void bonk::BinaryExportMainStageCallback::operator()(bonk::OperatorType& value, std::string_view) {
    context.stream.get_stream().write((char*)&value, sizeof(value));
}
void bonk::BinaryExportMainStageCallback::operator()(bonk::NumberConstantContents& value,
                                                std::string_view) {
    context.stream.get_stream().write((char*)&value, sizeof(value));
}
void bonk::BinaryExportMainStageCallback::operator()(bonk::TrivialTypeKind& value, std::string_view) {
    context.stream.get_stream().write((char*)&value, sizeof(value));
}
void bonk::BinaryExportMainStageCallback::operator()(bonk::ParserPosition& value, std::string_view) {
    context.write_string(value.filename);
    context.stream.get_stream().write((char*)&value.line, sizeof(value.line));
    context.stream.get_stream().write((char*)&value.ch, sizeof(value.ch));
    context.stream.get_stream().write((char*)&value.index, sizeof(value.index));
}

void bonk::BinaryExportMainStageCallback::operator()(std::string_view value, std::string_view) {
    context.write_string(value);
}

void bonk::BinaryExportMainStageCallback::operator()(TreeNodeType& value, std::string_view) {
    context.stream.get_stream().write((char*)&value, sizeof(value));
}

void bonk::BinaryExportContext::register_string(std::string_view value) {
    if(value.size() < sizeof(unsigned long) - 1) {
        return;
    }

    auto it = string_positions.find(value);
    if (it == string_positions.end()) {
        string_positions[value] = string_header_length;
        string_header_length += value.length() + 1;
        string_header.push_back(value);
    }
}

void bonk::BinaryExportContext::dump_string_header() {
    stream.get_stream().write((char*)&string_header_length, sizeof(string_header_length));
    for (auto& string : string_header) {
        stream.get_stream() << string;
        stream.get_stream() << (char)0;
    }
}
void bonk::BinaryExportContext::write_string(std::string_view value) {
    if(value.size() < sizeof(unsigned long) - 1) {
        unsigned long length = value.length();
        stream.get_stream() << (unsigned char)1;
        stream.get_stream() << value;
        stream.get_stream() << (unsigned char)0;
        return;
    }
    stream.get_stream() << (unsigned char)0;
    // Find string in string table
    auto it = string_positions.find(value);
    assert(it != string_positions.end());
    stream.get_stream().write((char*)&it->second, sizeof(it->second));
}
