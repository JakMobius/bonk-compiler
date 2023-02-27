
#pragma once

#include "../parser.hpp"
#include "grammatic_expression.hpp"

namespace bonk {

TreeNodeList<TreeNode*>* parse_grammatic_block(Parser* parser);

}
