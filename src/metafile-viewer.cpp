
#include <fstream>
#include "argparse/argparse.hpp"
#include "bonk/backend/qbe/qbe_backend.hpp"
// #include "bonk/backend/x86/x86_backend.hpp"
#include "bonk/compiler.hpp"
#include "bonk/help-resolver/help_resolver.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/ast_printer.hpp"
#include "bonk/tree/json_ast_serializer.hpp"

struct InitErrorReporter {

    bonk::StdOutputStream error_file{std::cerr};

    [[nodiscard]] bonk::MessageStreamProxy warning() const {
        return {bonk::CompilerMessageType::warning, error_file};
    }

    [[nodiscard]] bonk::MessageStreamProxy error() const {
        return {bonk::CompilerMessageType::error, error_file};
    }

    [[nodiscard]] bonk::MessageStreamProxy fatal_error() const {
        return {bonk::CompilerMessageType::fatal_error, error_file};
    }
};

int main(int argc, const char* argv[]) {

    InitErrorReporter error_reporter;

    argparse::ArgumentParser program("bonk");
    program.add_argument("input").help("path to the source file to view metadata from");
    program.add_argument("-h", "--help")
        .default_value(false)
        .implicit_value(true)
        .help("show this help message and exit");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    std::filesystem::path input_file_path = program.get<std::string>("input");
    const auto help_flag = program.get<bool>("--help");

    if (help_flag || input_file_path.empty()) {
        std::cout << program;
        return EXIT_SUCCESS;
    }

    bonk::Compiler compiler;
    bonk::MiddleEnd middle_end(compiler);
    bonk::SourceMetadata metadata{middle_end, input_file_path};

    if (!metadata.get_meta_ast()) {
        std::cerr << "Could not read metafile for " << input_file_path.string() << "\n";
        return EXIT_FAILURE;
    }

    std::cout << "Meta AST for the file:\n\n";

    bonk::StdOutputStream output_stream{std::cout};
    bonk::ASTPrinter printer(output_stream);
    metadata.get_meta_ast()->accept(&printer);

    std::cout << "\nExternal symbols for the file:\n\n";

    metadata.fill_external_symbol_table(middle_end);

    struct IdentifierVisitor : public bonk::ASTVisitor {
        bonk::MiddleEnd& middle_end;

        IdentifierVisitor(bonk::MiddleEnd& middle_end): middle_end(middle_end) {}

        void visit(bonk::TreeNodeIdentifier* node) override {
            auto file_index = middle_end.external_symbol_table.external_symbol_def_files[node];
            auto& file = middle_end.external_symbol_table.external_files[file_index];

            std::cout << "Identifier " << node->identifier_text << " at line " << node->source_position.line
                      << " is defined at " << std::string_view{file.data(), file.size()} << "\n";
        }
    };

    IdentifierVisitor visitor{middle_end};
    metadata.get_meta_ast()->accept(&visitor);

    return EXIT_SUCCESS;
}
