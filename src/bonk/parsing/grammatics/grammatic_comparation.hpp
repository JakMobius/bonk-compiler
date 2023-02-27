
#pragma once

#include "../parser.hpp"
#include "grammatic_math_expression.hpp"

namespace bonk {

bool is_comparation_operator(OperatorType oper);

TreeNode* parse_grammatic_comparation(Parser* parser);

} // namespace bonk
