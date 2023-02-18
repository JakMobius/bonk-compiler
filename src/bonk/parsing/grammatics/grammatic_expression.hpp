#pragma once

#include "../parser.hpp"
#include "../../tree/nodes/nodes.hpp"
#include "grammatic_assignment.hpp"
#include "grammatic_logic_expression.hpp"
#include "grammatic_sub_block.hpp"
#include "grammatic_unary_operator.hpp"
#include "grammatic_var_definition.hpp"

namespace bonk {

tree_node* parse_grammatic_expression(parser* parser);

tree_node* parse_grammatic_expression_leveled(parser* parser, bool top_level);

} // namespace bonk