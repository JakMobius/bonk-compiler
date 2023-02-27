
#pragma once

#include "../parser.hpp"

namespace bonk {

TreeNode* parse_grammatic_sub_block(Parser* parser);

TreeNodeList<TreeNode*>* parse_grammatic_nested_block(Parser* parser);

TreeNodeCycle* parse_grammatic_cycle(Parser* parser);

TreeNodeCheck* parse_grammatic_check(Parser* parser);

} // namespace bonk
