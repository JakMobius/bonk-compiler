
#pragma once

#include "../parser.hpp"
#include "grammatic_assignment.hpp"
#include "grammatic_comparation.hpp"
#include "grammatic_unary_operator.hpp"

namespace bonk {

tree_node* parse_grammatic_logic_term(parser* parser);

}