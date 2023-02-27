#pragma once

#include "../parser.hpp"
#include "./grammatic_program.hpp"
#include "grammatic_help.hpp"

namespace bonk {

bool parse_grammatic_global(Parser* parser, TreeNodeList<TreeNode*>* target);

}
