
#include "node.hpp"

namespace bonk {

void TreeNode::serialize(JsonSerializer* serializer) {
    if (source_position) {
        const char* position_string = source_position->to_string();
        serializer->block_string_field("source_position", position_string);
        free((void*)position_string);
    }
}

}; // namespace bonk