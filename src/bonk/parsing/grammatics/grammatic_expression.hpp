#pragma once

#include "../parser.hpp"
#include "../../tree/ast.hpp"
#include "grammatic_assignment.hpp"
#include "grammatic_logic_expression.hpp"
#include "grammatic_sub_block.hpp"
#include "grammatic_unary_operator.hpp"
#include "grammatic_var_definition.hpp"

namespace bonk {

TreeNode* parse_grammatic_expression(Parser* parser);

TreeNode* parse_grammatic_expression_leveled(Parser* parser, bool top_level);

} // namespace bonk
