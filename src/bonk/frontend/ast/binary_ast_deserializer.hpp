
#pragma once

#include <cassert>
#include <sstream>
#include <unordered_map>
#include "ast.hpp"
#include "ast_field_walker.hpp"
#include "ast_visitor.hpp"
#include "node_field_walker.hpp"
#include "template_visitor.hpp"
#include "utils/streams.hpp"

namespace bonk {

struct BinaryImportContext {
    ASTVisitor* visitor = nullptr;
    const BufferInputStream& stream;
    const char* string_table = nullptr;

    BinaryImportContext(const BufferInputStream& stream) : stream(stream) {
        string_table = stream.input.data() + stream.tell();
    }

    std::string_view read_string();
};

class BinaryImportMainStageCallback {

    BinaryImportContext& context;

  public:
    BinaryImportMainStageCallback(BinaryImportContext& context) : context(context) {
    }

    void operator()(bonk::OperatorType& value, std::string_view);
    void operator()(bonk::NumberConstantContents& value, std::string_view);
    void operator()(bonk::TrivialTypeKind& value, std::string_view);
    void operator()(bonk::ParserPosition& value, std::string_view);
    void operator()(std::string_view& value, std::string_view);
    void operator()(std::string& value, std::string_view);
    void operator()(TreeNodeType& value, std::string_view);

    template <typename T> void operator()(std::unique_ptr<T>& value, std::string_view) {
        char is_present = 0;
        context.stream.get_stream().read(&is_present, sizeof(char));

        if (is_present) {
            value = std::unique_ptr<T>((T*)read_node().release());
        } else {
            value = nullptr;
        }
    }

    template <typename T>
    void operator()(std::list<std::unique_ptr<T>>& value, std::string_view name) {
        unsigned int length = 0;
        context.stream.get_stream().read((char*)&length, sizeof(length));

        value.clear();
        for (int i = 0; i < length; i++) {
            char is_present = 0;
            context.stream.get_stream().read(&is_present, sizeof(is_present));

            if (is_present) {
                value.push_back(std::unique_ptr<T>((T*)read_node().release()));
            } else {
                value.push_back(std::unique_ptr<T>(nullptr));
            }
        }
    }

    std::unique_ptr<bonk::TreeNode> read_node();
};

struct BinaryASTDeserializer {
    explicit BinaryASTDeserializer(const bonk::BufferInputStream& stream) : stream(stream) {
    }

    std::unique_ptr<TreeNode> read();

    const BufferInputStream& stream;
};

} // namespace bonk