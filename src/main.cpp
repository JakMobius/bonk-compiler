
#include <fstream>
#include <sys/stat.h>
#include "argparse/argparse.hpp"
#include "bonk/backend/qbe/qbe_backend.hpp"
#include "bonk/backend/x86/x86_backend.hpp"
#include "bonk/compiler.hpp"
#include "bonk/middleend/ir/hir_printer.hpp"
#include "bonk/middleend/middleend.hpp"
#include "bonk/tree/ast_printer.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

struct InitErrorReporter {

    bonk::StdOutputStream error_file{std::cerr};

    bonk::MessageStreamProxy warning() const {
        return {bonk::CompilerMessageType::warning, error_file};
    }

    bonk::MessageStreamProxy error() const {
        return {bonk::CompilerMessageType::error, error_file};
    }

    bonk::MessageStreamProxy fatal_error() const {
        return {bonk::CompilerMessageType::fatal_error, error_file};
    }
};

bool dump_ast(bonk::TreeNode* ast, std::string_view mode, bonk::OutputStream& output_file) {
    if (mode == "json") {
        bonk::JsonSerializer serializer{output_file};
        bonk::JsonDumpAstVisitor visitor{serializer};
        ast->accept(&visitor);
    } else if (mode == "code") {
        bonk::ASTPrinter printer{output_file};
        ast->accept(&printer);
    } else {
        std::cerr << "Unknown AST dump mode: " << mode << std::endl;
        return false;
    }
    return true;
}

int main(int argc, const char* argv[]) {

    InitErrorReporter error_reporter;

    argparse::ArgumentParser program("bonk");
    program.add_argument("input").help("path to the input file");
    program.add_argument("-h", "--help")
        .default_value(false)
        .implicit_value(true)
        .help("show this help message and exit");
    program.add_argument("--ast").implicit_value(std::string("code")).help(
        "output AST ('json' or 'code' - default) ");
    program.add_argument("--ast-late")
        .implicit_value(std::string("code"))
        .help("output AST after middleend (outputs latest AST available, even after errors) "
              "('json' or 'code' - default)");
    program.add_argument("--hir").default_value(false).implicit_value(true).help(
        "output intermediate representation");
    program.add_argument("-o", "--output-file")
        .default_value(std::string("out"))
        .nargs(1)
        .help("path to the output file");
    program.add_argument("-t", "--target")
        .default_value(std::string("qbe"))
        .nargs(1)
        .help("compile target (qbe)");
    program.add_argument("-l", "--listing-file").nargs(1).help("path to the listing file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    const auto input_file_path = program.get<std::string>("input");
    const auto help_flag = program.get<bool>("--help");
    const auto hir_flag = program.get<bool>("--hir");
    const auto output_file_path = program.get<std::string>("--output-file");
    const auto target_flag = program.get<std::string>("--target");

    if (help_flag || input_file_path.empty()) {
        std::cout << program;
        return EXIT_SUCCESS;
    }

    std::unique_ptr<bonk::OutputStream> error_file =
        std::make_unique<bonk::StdOutputStream>(std::cerr);
    std::unique_ptr<bonk::OutputStream> output_file =
        std::make_unique<bonk::FileOutputStream>(output_file_path);
    std::unique_ptr<bonk::OutputStream> listing_file = std::make_unique<bonk::NullOutputStream>();
    std::unique_ptr<bonk::InputStream> input_file;

    if (!output_file->get_stream()) {
        error_reporter.fatal_error() << "could not open output file '" << output_file_path << "'";
        return EXIT_FAILURE;
    }

    input_file = std::make_unique<bonk::FileInputStream>(input_file_path);
    if (!input_file->get_stream()) {
        error_reporter.fatal_error() << "could not open input file '" << input_file_path << "'";
        return EXIT_FAILURE;
    }

    if (auto log_file = program.present("--listing-file")) {
        listing_file = std::make_unique<bonk::FileOutputStream>(log_file.value());
        if (!listing_file->get_stream()) {
            error_reporter.warning() << "could not open listing file '" << log_file.value() << "'";
            listing_file = std::make_unique<bonk::NullOutputStream>();
        }
    }

    bonk::CompilerConfig config = {
        .error_file = *error_file,
        .listing_file = *listing_file,
        .output_file = *output_file,
    };

    bonk::Compiler compiler(config);
    std::unique_ptr<bonk::Backend> backend;

    if (target_flag == "x86") {
        backend = std::make_unique<bonk::x86_backend::Backend>(compiler);
    } else if (target_flag == "qbe") {
        backend = std::make_unique<bonk::qbe_backend::QBEBackend>(compiler);
    } else {
        error_reporter.fatal_error() << "unknown compile target: '" << target_flag.c_str() << "'";
        return 1;
    }

    std::string source;
    source.assign((std::istreambuf_iterator<char>(input_file->get_stream())),
                  std::istreambuf_iterator<char>());

    auto lexemes =
        bonk::LexicalAnalyzer(compiler).parse_file(input_file_path.c_str(), source.c_str());

    if (compiler.state) {
        return EXIT_FAILURE;
    }

    auto ast = bonk::Parser(compiler).parse_file(&lexemes);

    if (compiler.state || !ast) {
        return EXIT_FAILURE;
    }

    if (auto ast_flag = program.present("--ast")) {
        if (dump_ast(ast.get(), ast_flag.value(), *output_file)) {
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    }

    // Run middle-end and back-end
    bonk::MiddleEnd middle_end(compiler);
    middle_end.transform_ast(ast.get());

    if (auto ast_flag = program.present("--ast-late")) {
        if (dump_ast(ast.get(), ast_flag.value(), *output_file)) {
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    }

    if (compiler.state) {
        return EXIT_FAILURE;
    }

    auto ir_program = middle_end.generate_hir(ast.get());

    if (!ir_program) {
        return EXIT_FAILURE;
    }

    if (hir_flag) {
        bonk::HIRPrinter printer{*output_file};
        printer.print(*ir_program);
    } else {
        backend->compile_program(*ir_program);

        if (chmod(output_file_path.c_str(), 511) < 0) {
            compiler.warning() << "failed to add execution permissions to file";
        }
    }

    if (compiler.state) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
