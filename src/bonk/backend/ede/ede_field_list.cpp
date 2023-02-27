
#include "ede_field_list.hpp"

namespace bonk::ede_backend {

FieldList::FieldList() {
    byte_offset = 0;
}

Variable* FieldList::get_variable(TreeNodeIdentifier* identifier) {

    for (int i = 0; i < variables.size(); i++) {
        if (variables[i]->identifier->contents_equal(identifier)) {
            return variables[i];
        }
    }

    return nullptr;
}

bool FieldList::add_variable(Variable* variable) {
    variable->byte_offset = frame_size + byte_offset;

    frame_size += 8;
    variables.push_back(variable);

    return true;
}

FieldList::~FieldList() {
    for (int i = 0; i < variables.size(); i++) {
        delete variables[i];
    }
}

} // namespace bonk::ede_backend