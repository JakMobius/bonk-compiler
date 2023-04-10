
#include <fstream>
#include "argparse/argparse.hpp"
#include "bonk/backend/qbe/qbe_backend.hpp"
//#include "bonk/backend/x86/x86_backend.hpp"
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

bool dump_ast(bonk::TreeNode* ast, std::string_view mode, bonk::OutputStream& output_file) {
    if (mode == "json") {

        bonk::JSONSerializer serializer{output_file};
        bonk::JSONASTSerializer ast_serializer{serializer};
        ast->accept(&ast_serializer);

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
    program.add_argument("-t", "--target")
        .default_value(std::string("qbe"))
        .nargs(1)
        .help("compile target (qbe)");
    program.add_argument("-g", "--debug")
            .default_value(false)
            .implicit_value(true)
            .help("generate debug symbols");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    std::filesystem::path input_file_path = program.get<std::string>("input");
    const auto help_flag = program.get<bool>("--help");
    const auto target_flag = program.get<std::string>("--target");

    if (help_flag || input_file_path.empty()) {
        std::cout << program;
        return EXIT_SUCCESS;
    }

    std::unique_ptr<bonk::OutputStream> error_file =
        std::make_unique<bonk::StdOutputStream>(std::cerr);

    bonk::CompilerConfig config = {
        .error_file = *error_file
    };

    bonk::Compiler compiler(config);
    std::unique_ptr<bonk::Backend> backend;

    if (target_flag == "x86") {
//        backend = std::make_unique<bonk::x86_backend::Backend>(compiler);
    } else if (target_flag == "qbe") {
        auto qbe_backend = std::make_unique<bonk::qbe_backend::QBEBackend>(compiler);
        if(program.get<bool>("--debug")) {
            qbe_backend->generate_debug_symbols = true;
        }
        backend = std::move(qbe_backend);
    } else {
        error_reporter.fatal_error() << "unknown compile target: '" << target_flag.c_str() << "'";
        return 1;
    }

    compiler.backend = backend.get();

    bonk::HelpResolver help_resolver{compiler};

    if(!help_resolver.compile_file(input_file_path)) {
        return 1;
    }

    std::filesystem::path bs_cache_path = input_file_path.parent_path() /= ".bscache";
    std::filesystem::path project_meta_path = bs_cache_path / input_file_path.stem() += ".project.meta";

    std::filesystem::create_directories(bs_cache_path);

    bonk::FileOutputStream project_meta_file{project_meta_path.string()};

    for(auto& file : compiler.output_files) {
        project_meta_file.get_stream() << file << "\n";
    }

    return EXIT_SUCCESS;
}
