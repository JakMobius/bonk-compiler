#pragma once

#include "../../compiler.hpp"
#include "ede_scope_stack.hpp"

namespace bonk::ede_backend {

struct backend_context {
    compiler* linked_compiler;
    scope_stack scope_stack;
    unsigned long processor_stack_usage;
    unsigned long labels;
    unsigned long cycle_label_id;
    unsigned long scope_stack_offset;
    FILE* target;

    backend_context(compiler* compiler, FILE* destination);

    void field_list_declare_variable(tree_node_variable_definition* node);

    void field_list_declare_block(tree_node_block_definition* node);

    void error_already_defined(tree_node_identifier* identifier);

    void error_undefined_reference(tree_node_identifier* node);

    const char* get_comparsion_instruction(operator_type operator_type);

    const char* get_comparsion_instruction_negated(operator_type operator_type);

    void reference_variable(tree_node_identifier* identifier);

    void compile_term(tree_node* node);

    void compile_math_expression(tree_node* node);

    void compile_factor(tree_node* node);

    void compile_math_comparsion(tree_node* node);

    void compile_expression(tree_node* node, unsigned long stack_bytes_expected);

    void compile_logic(tree_node_operator* oper, unsigned long stack_bytes_expected);

    void compile_check(tree_node_check* node, unsigned long stack_bytes_expected);

    void compile_cycle(tree_node_cycle* node, unsigned long stack_bytes_expected);

    void compile_assignment(tree_node_identifier* variable, tree_node* value,
                            unsigned long stack_bytes_expected);

    void compile_rebonk_statement(tree_node_operator* node, unsigned long stack_bytes_expected);

    void compile_bonk_statement(tree_node_operator* node, unsigned long stack_bytes_expected);

    void compile_brek_statement(tree_node_operator* node, unsigned long stack_bytes_expected);

    void compile_inline_assembly(tree_node_operator* node);

    void compile_call(tree_node_call* call);

    void compile_block(tree_node_list<tree_node*>*, bool reset_frame_offset);

    void compile_callable_block(tree_node_block_definition* node);

    field_list* read_scope_variables(tree_node_list<tree_node*>*, bool reset_frame_offset);

    field_list* field_list_find_block_parameters(tree_node_block_definition* block);

    unsigned long next_label_id();

    void compile_program(tree_node_list<tree_node*>* node);

    tree_node*
    call_argument_list_get_value(tree_node_list<tree_node_call_parameter*>* argument_list,
                                 tree_node_identifier* identifier);

    void compile_print(tree_node_operator* node);
};

} // namespace bonk::ede_backend
