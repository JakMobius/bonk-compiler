
#include "ede_field_list.hpp"

namespace bonk::ede_backend {

field_list::field_list() {
    byte_offset = 0;
}

variable* field_list::get_variable(tree_node_identifier* identifier) {

    for (int i = 0; i < variables.size(); i++) {
        if (variables[i]->identifier->contents_equal(identifier)) {
            return variables[i];
        }
    }

    return nullptr;
}

bool field_list::add_variable(variable* variable) {
    variable->byte_offset = frame_size + byte_offset;

    frame_size += 8;
    variables.push_back(variable);

    return true;
}

field_list::~field_list() {
    for (int i = 0; i < variables.size(); i++) {
        delete variables[i];
    }
}

} // namespace bonk::ede_backend