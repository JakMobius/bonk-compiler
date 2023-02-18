
#pragma once

#include "../parser.hpp"
#include "grammatic_math_expression.hpp"

namespace bonk {

bool is_comparation_operator(operator_type oper);

tree_node* parse_grammatic_comparation(parser* parser);

} // namespace bonk
