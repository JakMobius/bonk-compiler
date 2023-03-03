
#include "compiler.hpp"

namespace bonk {

Compiler::Compiler(const CompilerConfig& config)
    : config(config), parser(*this), lexical_analyzer(this) {
}

} // namespace bonk