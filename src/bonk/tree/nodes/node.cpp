
#include "node.hpp"

namespace bonk {

void tree_node::serialize(json_serializer* serializer) {
    if (source_position) {
        const char* position_string = source_position->to_string();
        serializer->block_string_field("source_position", position_string);
        free((void*)position_string);
    }
}

}; // namespace bonk