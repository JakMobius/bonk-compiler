
#include "bonk_cli_arguments.hpp"

namespace args {

const flag TYPES[] = {{ARG_TYPE_FLAG, "h", "prints help page"},
                      {ARG_TYPE_KEY, "o", "output file (defaults to ./out"},
                      {ARG_TYPE_FLAG, "a", "output AST tree instead of binary"},
                      {ARG_TYPE_KEY, "t", "compile target (ede / x86)"},
                      {ARG_TYPE_KEY, "l", "log file"},
                      {arg_type(0), nullptr, nullptr}};

const char* HELP_HEADER = "Usage: bs <input file> [flags]";

} // namespace args