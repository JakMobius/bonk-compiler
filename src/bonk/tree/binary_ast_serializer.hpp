#pragma once

#include <unordered_map>
#include "ast.hpp"
#include "ast_field_walker.hpp"
#include "ast_visitor.hpp"
#include "node_field_walker.hpp"
#include "template_visitor.hpp"
#include "utils/streams.hpp"

namespace bonk {

struct BinaryExportContext {
    ASTVisitor* visitor = nullptr;
    const OutputStream& stream;
    std::vector<std::string_view> string_header;
    std::unordered_map<std::string_view, unsigned long> string_positions;
    unsigned int string_header_length = sizeof(unsigned int);

    BinaryExportContext(const OutputStream& stream) : stream(stream) {
    }

    void write_string(std::string_view value);
    void register_string(std::string_view value);
    void dump_string_header();
};

class BinaryExportMainStageCallback {

    BinaryExportContext& context;

  public:
    BinaryExportMainStageCallback(BinaryExportContext& context) : context(context) {
    }

    void operator()(bonk::OperatorType& value, std::string_view);
    void operator()(bonk::NumberConstantContents& value, std::string_view);
    void operator()(bonk::TrivialTypeKind& value, std::string_view);
    void operator()(bonk::ParserPosition& value, std::string_view);
    void operator()(std::string_view value, std::string_view);
    void operator()(TreeNodeType& value, std::string_view);

    template <typename T> void operator()(std::unique_ptr<T>& value, std::string_view) {
        if (value) {
            context.stream.get_stream() << (char)1;
            value->accept(context.visitor);
        } else {
            context.stream.get_stream() << (char)0;
        }
    }

    template <typename T>
    void operator()(std::list<std::unique_ptr<T>>& value, std::string_view name) {
        unsigned int length = value.size();
        context.stream.get_stream().write((char*)&length, sizeof(length));

        for (auto& element : value) {
            if (element) {
                context.stream.get_stream() << (char)1;
                element->accept(context.visitor);
            } else {
                context.stream.get_stream() << (char)0;
            }
        }
    }
};

class BinaryExportStringStageCallback {

    BinaryExportContext& context;

  public:
    BinaryExportStringStageCallback(BinaryExportContext& context) : context(context) {
    }

    void operator()(ParserPosition& value, std::string_view) {
        context.register_string(value.filename);
    }

    void operator()(std::string_view value, std::string_view) {
        context.register_string(value);
    }

    void operator()(std::string& value, std::string_view) {
        context.register_string(value);
    }

    template <typename T> void operator()(std::unique_ptr<T>& value, std::string_view) {
        if (value) {
            value->accept(context.visitor);
        }
    }

    template <typename T>
    void operator()(std::list<std::unique_ptr<T>>& value, std::string_view name) {
        for (auto& element : value) {
            if (element) {
                element->accept(context.visitor);
            }
        }
    }

    template <typename T> void operator()(T&, std::string_view) {
    }
};

struct BinaryASTSerializer : public bonk::TemplateVisitor<BinaryASTSerializer> {
    explicit BinaryASTSerializer(const bonk::OutputStream& stream)
        : stream(stream), TemplateVisitor(*this) {
    }

    template <typename T> void operator()(T& node) {
        // Setup context and export stages
        BinaryExportContext export_context(stream);
        BinaryExportStringStageCallback string_export_callback(export_context);
        BinaryExportMainStageCallback field_callback(export_context);

        bonk::ASTFieldWalker string_export_walker(string_export_callback);
        bonk::ASTFieldWalker field_export_walker(field_callback);

        export_context.visitor = &string_export_walker;
        node->accept(&string_export_walker);

        export_context.dump_string_header();

        export_context.visitor = &field_export_walker;
        node->accept(&field_export_walker);
    }

    const OutputStream& stream;
};

} // namespace bonk