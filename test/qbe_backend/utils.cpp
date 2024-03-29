
#include "utils.hpp"
#include "bonk/middleend/ir/algorithms/hir_base_block_separator.hpp"
#include "bonk/middleend/ir/algorithms/hir_block_sorter.hpp"
#include "bonk/middleend/ir/algorithms/hir_copy_propagation.hpp"
#include "bonk/middleend/ir/algorithms/hir_jmp_reducer.hpp"
#include "bonk/middleend/ir/algorithms/hir_jnz_optimizer.hpp"
#include "bonk/middleend/ir/algorithms/hir_loc_collapser.hpp"
#include "bonk/middleend/ir/algorithms/hir_ref_count_replacer.hpp"
#include "bonk/middleend/ir/algorithms/hir_ref_counter_reducer.hpp"
#include "bonk/middleend/ir/algorithms/hir_ssa_converter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unreachable_code_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_unused_def_deleter.hpp"
#include "bonk/middleend/ir/algorithms/hir_variable_index_compressor.hpp"
#include "bonk/middleend/ir/hir_printer.hpp"
#include "bonk/middleend/middleend.hpp"

bool run_bonk(const char* bonk_source, const char* executable_name, const RunParameters& parameters) {
    if (!compile_bonk_source(bonk_source, "bonk.qbe", parameters))
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
                               const char* executable_name, const RunParameters& parameters) {
    if (!compile_c_source(c_source, "c_counterpart.o"))
        return false;
    if (!compile_bonk_source(bonk_source, "bonk.qbe", parameters))
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

bool compile_bonk_source(const char* source, std::filesystem::path output_file, const RunParameters& parameters) {
    ensure_path(output_file);

    auto error_stream = bonk::StdOutputStream(std::cout);
    auto output_stream = bonk::FileOutputStream(output_file.string());

    bonk::CompilerConfig config{.error_file = error_stream };
    bonk::Compiler compiler(config);

    auto lexemes = bonk::Lexer(compiler).parse_file("test", source);

    if (lexemes.empty()) {
        return false;
    }

    auto root = bonk::Parser(compiler).parse_file(&lexemes);

    if (root == nullptr) {
        return false;
    }

    auto ast = bonk::AST();
    ast.root = std::move(root);

    bonk::FrontEnd front_end(compiler);

    if (!front_end.transform_ast(ast)) {
        return false;
    }

    auto ir_program = front_end.generate_hir(ast.root.get());

    if (ir_program == nullptr) {
        return false;
    }

    bonk::HIRVariableIndexCompressor().compress(*ir_program);
    bonk::HIRBaseBlockSeparator().separate_blocks(*ir_program);
    bonk::HIRLocCollapser().collapse(*ir_program);

//    bonk::StdOutputStream output(std::cout);
//    bonk::HIRPrinter(output).print(*ir_program);

    // <optimizations>
    bonk::HIRSSAConverter().convert(*ir_program);
    bonk::HIRCopyPropagation().propagate_copies(*ir_program);
    bonk::HIRUnusedDefDeleter().delete_unused_defs(*ir_program);

    if(parameters.optimize_reference_counter) {
        bonk::HIRRefCountReducer().reduce(*ir_program);
    }
    // </optimizations>

    bonk::HIRRefCountReplacer().replace_ref_counters(*ir_program);

    // <optimizations>
    bonk::HIRJnzOptimizer().optimize(*ir_program);

    bonk::HIRUnreachableCodeDeleter().delete_unreachable_code(*ir_program);
    bonk::HIRJmpReducer().reduce(*ir_program);

    bonk::HIRUnusedDefDeleter().delete_unused_defs(*ir_program);
    bonk::HIRVariableIndexCompressor().compress(*ir_program);
    bonk::HIRLocCollapser().collapse(*ir_program);
    bonk::HIRBlockSorter().sort(*ir_program);
    // </optimizations>

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