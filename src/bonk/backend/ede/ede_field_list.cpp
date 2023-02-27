
#include "ede_field_list.hpp"

namespace bonk::ede_backend {

FieldList::FieldList() {
    byte_offset = 0;
}

Variable* FieldList::get_variable(TreeNodeIdentifier* identifier) {

    for (auto & variable : variables) {
        if (variable->identifier->contents_equal(identifier)) {
            return variable;
        }
    }

    return nullptr;
}

void FieldList::add_variable(Variable* variable) {
    variable->byte_offset = frame_size + byte_offset;

    frame_size += 8;
    variables.push_back(variable);
}

FieldList::~FieldList() {
    for (auto & variable : variables) {
        delete variable;
    }
}

} // namespace bonk::ede_backend