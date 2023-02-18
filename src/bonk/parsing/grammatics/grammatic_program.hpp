
#pragma once

#include "../parser.hpp"
#include "./grammatic_block_definition.hpp"

namespace bonk {

bool parse_grammatic_program(parser* parser, tree_node_list<tree_node*>* target);

}
