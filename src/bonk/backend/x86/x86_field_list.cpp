
#include "x86_field_list.hpp"

namespace bonk::x86_backend {

FieldList::FieldList(RegisterDescriptorList* descriptor_list, BackendContextState* state) {
    descriptors = descriptor_list;
    context_state = state;
    arguments = 0;
}

Variable* FieldList::get_variable(TreeNodeIdentifier* identifier) {

    for (int i = 0; i < variables.size(); i++) {
        if (variables[i]->identifier->contents_equal(identifier)) {
            return variables[i];
        }
    }

    return nullptr;
}

bool FieldList::add_variable(Variable* variable) const {
    CommandList* list = nullptr;
    if (context_state)
        list = context_state->current_command_list;
    //    if(variable->is_contextual) {
    //        if(arguments < SYSTEM_V_ARGUMENT_REGISTERS_COUNT) {
    //            variable->storage =
    //            descriptors->next_constrained_register(SYSTEM_V_ARGUMENT_REGISTERS[arguments],
    //            list); arguments++;
    //        } else {
    //            variable->storage = descriptors->next_register(list);
    //        }
    //    } else {
    variable->storage = descriptors->next_register(list);
    //    }
    variables.push_back(variable);
    return true;
}

FieldList::~FieldList() {
    for (int i = 0; i < variables.size(); i++) {
        delete variables[i];
    }
}

} // namespace bonk::x86_backend