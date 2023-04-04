#pragma once

#include <sstream>
#include "ast.hpp"
#include "ast_field_walker.hpp"
#include "ast_visitor.hpp"
#include "node_field_walker.hpp"
#include "template_visitor.hpp"
#include "utils/json_serializer.hpp"

namespace bonk {

class JSONASTFieldCallback {

    JSONSerializer& serializer;
    ASTVisitor* visitor;

  public:
    JSONASTFieldCallback(JSONSerializer& serializer, ASTVisitor* visitor)
        : serializer(serializer), visitor(visitor) {
    }

    void operator()(bonk::OperatorType& value, std::string_view name);
    void operator()(bonk::NumberConstantContents& value, std::string_view name);
    void operator()(bonk::TrivialTypeKind& value, std::string_view name);
    void operator()(bonk::ParserPosition& value, std::string_view name);
    void operator()(std::string_view value, std::string_view name);
    void operator()(TreeNodeType& value, std::string_view name);

    template <typename T> void operator()(std::unique_ptr<T>& value, std::string_view name) {
        if (value) {
            serializer.field(name).block_start_block();
            value->accept(visitor);
            serializer.close_block();
        } else
            serializer.field(name).block_add_null();
    }

    template <typename T>
    void operator()(std::list<std::unique_ptr<T>>& value, std::string_view name) {
        serializer.field(name).block_start_array();

        for (auto& element : value) {
            if (element) {
                serializer.array_add_block();
                element->accept(visitor);
                serializer.close_block();
            } else
                serializer.array_add_null();
        }

        serializer.close_array();
    }
};

struct JSONASTSerializer : public bonk::ASTFieldWalker<JSONASTFieldCallback> {
    explicit JSONASTSerializer(bonk::JSONSerializer& serializer)
        : field_callback(serializer, this), ASTFieldWalker(field_callback) {
    }

    JSONASTFieldCallback field_callback;
};

} // namespace bonk