
#pragma once

#include "../parser.hpp"
#include "grammatic_expression.hpp"

namespace bonk {

tree_node_list<tree_node*>* parse_grammatic_block(parser* parser);

}
