
#include "compiler.hpp"

namespace bonk {

Compiler::Compiler() : config() {
}

Compiler::Compiler(const CompilerConfig& config) : config(config) {
}

void Compiler::report_project_file(std::string_view path) {
    output_files.insert(std::string(path));
}

MessageStreamProxy Compiler::warning() const {
    return {CompilerMessageType::warning, config.error_file};
}

MessageStreamProxy Compiler::error() {
    return {CompilerMessageType::error, config.error_file};
}

MessageStreamProxy Compiler::fatal_error() {
    return {CompilerMessageType::fatal_error, config.error_file};
}

} // namespace bonk