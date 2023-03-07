#pragma once

#if 0

namespace bonk::x86_backend {

struct Variable;
struct VariableFunction;

enum VariableType { VARIABLE_TYPE_NUMBER, VARIABLE_TYPE_FUNCTION };

} // namespace bonk::x86_backend

#include "bonk/tree/ast.hpp"
#include "x86_abstract_register.hpp"
#include "x86_field_list.hpp"

namespace bonk::x86_backend {

struct Variable {
    bool is_contextual{};
    VariableType type;
    TreeNodeIdentifier* identifier;
    AbstractRegister storage{};

    Variable(TreeNodeIdentifier* identifier);
};

struct VariableNumber : Variable {
    VariableNumber(TreeNodeVariableDefinition* definition);
};

struct VariableFunction : Variable {
    FieldList* argument_list;

    VariableFunction(TreeNodeIdentifier* the_identifier, FieldList* the_argument_list);
};

} // namespace bonk::x86_backend

#endif
