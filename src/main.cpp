
#include <fstream>
#include <sys/stat.h>
#include "argparse/argparse.hpp"
#include "bonk/backend/ede/ede_backend.hpp"
#include "bonk/backend/x86/x86_backend.hpp"
#include "bonk/compiler.hpp"
#include "bonk/tree/json_dump_ast_visitor.hpp"

void init_fatal_error(const char* format, ...) {
    va_list ap;

    va_start(ap, format);
    fprintf(stderr, "fatal error: ");
    vfprintf(stderr, format, ap);
    va_end(ap);
    fputc('\n', stderr);
}

void warning(const char* reason) {
    fprintf(stderr, "warning: %s\n", reason);
}

int main(int argc, const char* argv[]) {

    argparse::ArgumentParser program("bonk");
    program.add_argument("input").help("path to the input file");
    program.add_argument("-h", "--help")
        .default_value(false)
        .implicit_value(true)
        .help("show this help message and exit");
    program.add_argument("--ast").default_value(false).implicit_value(true).help(
        "output AST as JSON");
    program.add_argument("-o", "--output-file")
        .default_value(std::string("out"))
        .nargs(1)
        .help("path to the output file");
    program.add_argument("-t", "--target")
        .default_value(std::string("ede"))
        .nargs(1)
        .help("compile target (ede or x86)");
    program.add_argument("-l", "--log-file").nargs(1).help("path to the log file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    const auto input = program.get<std::string>("input");
    const auto help_flag = program.get<bool>("--help");
    const auto ast_flag = program.get<bool>("--ast");
    const auto output_file_path = program.get<std::string>("--output-file");
    const auto target_flag = program.get<std::string>("--target");

    if (help_flag || input.empty()) {
        std::cout << program;
        return 0;
    }

    bonk::CompilerConfig config = {};
    config.error_file = stderr;

    if (target_flag == "x86") {
        config.compile_backend = new bonk::x86_backend::Backend();
    } else if (target_flag == "ede") {
        config.compile_backend = new bonk::ede_backend::Backend();
    } else {
        init_fatal_error("unknown compile target: %s", target_flag.c_str());
        return 1;
    }

    FILE* output_file = fopen(output_file_path.c_str(), "wb");
    if (!output_file) {
        init_fatal_error("failed to open input file\n");
        delete config.compile_backend;
        return 1;
    }

    if (auto log_file = program.present("--log-file")) {
        config.listing_file = fopen(log_file->c_str(), "w");
        if (!config.listing_file) {
            warning("failed to open log file\n");
            config.listing_file = nullptr;
        }
    }

    bonk::Compiler compiler(&config);

    std::string source;
    std::ifstream file_stream(input);
    if (!file_stream.is_open()) {
        init_fatal_error("failed to open input file\n");
        delete config.compile_backend;
        return 1;
    }

    source.assign((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>());

    auto lexemes = compiler.lexical_analyzer.parse_file(input.c_str(), source.c_str());
    auto ast = compiler.parser.parse_file(&lexemes);

    if (ast) {
        if (ast_flag) {
            JsonSerializer serializer{output_file};
            bonk::JsonDumpAstVisitor visitor{&serializer};
            ast->accept(&visitor);
        } else {
            compiler.compile_ast(ast.get(), output_file);
        }
    }

    fclose(output_file);

    if (!ast_flag) {
        if (chmod(output_file_path.c_str(), 511) < 0) {
            warning("failed to add execution permissions to file\n");
        }
    }

    delete config.compile_backend;

    if (compiler.state) {
        return 1;
    }

    return 0;
}
