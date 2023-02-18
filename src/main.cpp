#include "bonk/backend/ede/ede_backend.hpp"
#include "bonk/backend/x86/x86_backend.hpp"
#include "bonk/compiler.hpp"
#include "argparse/argparse.hpp"

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
    program.add_argument("input")
        .help("path to the input file");
    program.add_argument("-h", "--help")
        .default_value(false)
        .implicit_value(true)
        .help("show this help message and exit");
    program.add_argument("--ast")
        .default_value(false)
        .implicit_value(true)
        .help("output AST as JSON");
    program.add_argument("-o", "--output-file")
        .default_value("out")
        .help("path to the output file");
    program.add_argument("-t", "--target")
        .default_value("ede")
        .help("compile target (ede or x86)");
    program.add_argument("-l", "--log-file")
        .help("path to the log file");

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

    bonk::compiler_config config = {};
    config.error_file = stderr;

    if(target_flag == "x86") {
        config.compile_backend = new bonk::x86_backend::backend();
    } else if(target_flag == "ede") {
        config.compile_backend = new bonk::ede_backend::backend();
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

    bonk::compiler compiler(&config);

    bonk::tree_node_list<bonk::tree_node*>* ast = compiler.get_ast_of_file_at_path(input.c_str());

    if (ast) {
        if (ast_flag) {
            auto* serializer = new json_serializer(output_file);
            ast->serialize(serializer);
            delete serializer;
        } else {
            compiler.compile_ast(ast, output_file);
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
