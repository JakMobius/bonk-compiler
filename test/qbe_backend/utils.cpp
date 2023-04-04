
#include "utils.hpp"

bool run_bonk(const char* bonk_source, const char* executable_name) {
    if (!compile_bonk_source(bonk_source, "bonk.qbe"))
            return false;
    if (!compile_qbe("bonk.qbe", "bonk.s"))
            return false;
    if (!compile_gcc("bonk.s", "bonk.o"))
            return false;
    if (!link_executable({"bonk.o"}, executable_name))
            return false;
    return true;
}

bool run_bonk_with_counterpart(const char* bonk_source, const char* c_source,
                               const char* executable_name) {
    if (!compile_c_source(c_source, "c_counterpart.o"))
        return false;
    if (!compile_bonk_source(bonk_source, "bonk.qbe"))
        return false;
    if (!compile_qbe("bonk.qbe", "bonk.s"))
        return false;
    if (!compile_gcc("bonk.s", "bonk.o"))
        return false;
    if (!link_executable({"bonk.o", "c_counterpart.o"}, executable_name))
        return false;
    return true;
}

void ensure_path(std::filesystem::path& path) {
    auto current_test = ::testing::UnitTest::GetInstance()->current_test_info();

    std::string testsuite_name = current_test->test_suite_name();
    std::string test_name = current_test->name();

    path = std::filesystem::path("artifacts") / testsuite_name / test_name / path;

    // Create directories if they don't exist

    std::filesystem::create_directories(path.parent_path());
}

bool compile_bonk_source(const char* source, std::filesystem::path output_file) {
    ensure_path(output_file);

    auto error_stream = bonk::StdOutputStream(std::cout);
    auto output_stream = bonk::FileOutputStream(output_file.string());

    bonk::CompilerConfig config{.error_file = error_stream };
    bonk::Compiler compiler(config);

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);

    if (compiler.state != bonk::BONK_COMPILER_OK) {
        return false;
    }

    auto root = bonk::Parser(compiler).parse_file(&lexemes);

    if (root == nullptr) {
        return false;
    }

    auto ast = bonk::AST();
    ast.root = std::move(root);

    bonk::MiddleEnd middle_end(compiler);

    if (!middle_end.transform_ast(ast)) {
        return false;
    }

    auto ir_program = middle_end.generate_hir(ast.root.get());

    if (ir_program == nullptr) {
        return false;
    }

    bonk::qbe_backend::QBEBackend(compiler).compile_program(*ir_program, output_stream);

    return true;
}

bool compile_qbe(std::filesystem::path input_file, std::filesystem::path output_file) {
    ensure_path(output_file);
    ensure_path(input_file);

    std::stringstream command_stream;
    command_stream << "qbe -o " << output_file << " " << input_file;
    std::string command = command_stream.str();

    if (system(command.c_str()) != 0) {
        ADD_FAILURE() << "Failed to compile " << input_file << " with QBE";
        return false;
    }
    return true;
}

bool compile_gcc(std::filesystem::path input_file, std::filesystem::path output_file) {
    ensure_path(output_file);
    ensure_path(input_file);

    std::stringstream command_stream;
    command_stream << "gcc -c -o " << output_file << " " << input_file;
    std::string command = command_stream.str();

    if (system(command.c_str()) != 0) {
        ADD_FAILURE() << "Failed to compile " << input_file << " with GCC";
        return false;
    }
    return true;
}

bool compile_c_source(const char* source, std::filesystem::path output_file) {
    ensure_path(output_file);

    std::stringstream command_stream;
    command_stream << "gcc -c -x c -o " << output_file << " -";
    std::string command = command_stream.str();

    FILE* pipe = popen(command.c_str(), "w");
    fwrite(source, 1, strlen(source), pipe);

    if (pclose(pipe) != 0) {
        ADD_FAILURE() << "Failed to compile C source";
        return false;
    }
    return true;
}

bool link_executable(const std::vector<std::filesystem::path>& object_files,
                     std::filesystem::path output_file) {
    // Make sure there is a env variable $BONK_STDLIB_PATH
    auto bonk_stdlib_path = std::getenv("BONK_STDLIB_PATH");

    if (bonk_stdlib_path == nullptr) {
        ADD_FAILURE() << "Environment variable $BONK_STDLIB_PATH is not defined, "
                         "test program cannot be linked";
        return false;
    }

    ensure_path(output_file);

    std::stringstream command_stream;
    command_stream << "gcc -o " << output_file;
    for (auto& object_file : object_files) {
        std::filesystem::path object_file_path = object_file;
        ensure_path(object_file_path);
        command_stream << " " << object_file_path;
    }

    // Add bonk stdlib to link
    command_stream << " " << bonk_stdlib_path;

    std::string command = command_stream.str();

    if (system(command.c_str()) != 0) {
        ADD_FAILURE() << "Failed to link executable";
        return false;
    }
    return true;
}

int get_executable_return_code(std::filesystem::path executable_file) {
    ensure_path(executable_file);

    std::string command = executable_file.string();

    int return_code = system(command.c_str());

    return WEXITSTATUS(return_code);
}

std::string get_executable_output(std::filesystem::path executable_file) {
    ensure_path(executable_file);

    std::string command = executable_file.string();

    std::array<char, 128> buffer{};
    std::stringstream result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        ADD_FAILURE() << "Failed to run executable";
        return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result << buffer.data();
    }
    return result.str();
}