
#pragma once

#include <ctype.h>
#include "../parser.hpp"
#include "./grammatic_expression.hpp"

namespace bonk {

TreeNode* parse_grammatic_reference(Parser* parser);

TreeNodeList<TreeNodeCallParameter*>* parse_grammatic_arguments(Parser* parser);

} // namespace bonk
