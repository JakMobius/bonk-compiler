
#pragma once

#include <ctype.h>
#include "../parser.hpp"
#include "./grammatic_expression.hpp"

namespace bonk {

tree_node* parse_grammatic_reference(parser* parser);

tree_node_list<tree_node_call_parameter*>* parse_grammatic_arguments(parser* parser);

} // namespace bonk
