#pragma once

namespace bonk::ede_backend {

struct Variable;
struct VariableFunction;

enum VariableType { VARIABLE_TYPE_NUMBER, VARIABLE_TYPE_FUNCTION };

} // namespace bonk::ede_backend

#include "../../tree/nodes/identifier.hpp"
#include "ede_field_list.hpp"

namespace bonk::ede_backend {

struct Variable {
    VariableType type;
    TreeNodeIdentifier* identifier;
    unsigned long byte_offset{};

    Variable(TreeNodeIdentifier* identifier);
};

struct VariableNumber : Variable {
    VariableNumber(TreeNodeIdentifier* identifier);
};

struct VariableFunction : Variable {
    FieldList* argument_list;

    VariableFunction(TreeNodeIdentifier* identifier, FieldList* the_argument_list);
};

} // namespace bonk::ede_backend
