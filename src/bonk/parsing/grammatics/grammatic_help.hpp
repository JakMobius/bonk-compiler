
#pragma once

#include "../../../utils/file_io.hpp"
#include "../parser.hpp"
#include "errno.h"

namespace bonk {

bool parse_grammatic_help(parser* thou, tree_node_list<tree_node*>* target);

}
