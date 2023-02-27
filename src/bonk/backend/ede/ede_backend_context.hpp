#pragma once

#include "../../compiler.hpp"
#include "ede_scope_stack.hpp"

namespace bonk::ede_backend {

struct BackendContext {
    Compiler* linked_compiler;
    ScopeStack scope_stack;
    unsigned long processor_stack_usage;
    unsigned long labels;
    unsigned long cycle_label_id;
    unsigned long scope_stack_offset;
    FILE* target;

    BackendContext(Compiler* compiler, FILE* destination);

    void field_list_declare_variable(TreeNodeVariableDefinition* node) const;

    void field_list_declare_block(TreeNodeBlockDefinition* node) const;

    void error_already_defined(TreeNodeIdentifier* identifier) const;

    void error_undefined_reference(TreeNodeIdentifier* node) const;

    const char* get_comparsion_instruction(OperatorType operator_type);

    const char* get_comparsion_instruction_negated(OperatorType operator_type);

    void reference_variable(TreeNodeIdentifier* identifier) const;

    void compile_term(TreeNode* node);

    void compile_math_expression(TreeNode* node);

    void compile_factor(TreeNode* node);

    void compile_math_comparsion(TreeNode* node);

    void compile_expression(TreeNode* node, unsigned long stack_bytes_expected);

    void compile_logic(TreeNodeOperator* oper, unsigned long stack_bytes_expected);

    void compile_check(TreeNodeCheck* node, unsigned long stack_bytes_expected);

    void compile_cycle(TreeNodeCycle* node, unsigned long stack_bytes_expected);

    void compile_assignment(TreeNodeIdentifier* variable, TreeNode* value,
                            unsigned long stack_bytes_expected);

    void compile_rebonk_statement(TreeNodeOperator* node, unsigned long stack_bytes_expected) const;

    void compile_bonk_statement(TreeNodeOperator* node, unsigned long stack_bytes_expected);

    void compile_brek_statement(TreeNodeOperator* node, unsigned long stack_bytes_expected) const;

    void compile_inline_assembly(TreeNodeOperator* node);

    void compile_call(TreeNodeCall* call);

    void compile_block(TreeNodeList<TreeNode*>*, bool reset_frame_offset);

    void compile_callable_block(TreeNodeBlockDefinition* node);

    FieldList* read_scope_variables(TreeNodeList<TreeNode*>*, bool reset_frame_offset);

    FieldList* field_list_find_block_parameters(TreeNodeBlockDefinition* block) const;

    unsigned long next_label_id();

    void compile_program(TreeNodeList<TreeNode*>* node);

    TreeNode*
    call_argument_list_get_value(TreeNodeList<TreeNodeCallParameter*>* argument_list,
                                 TreeNodeIdentifier* identifier);

    void compile_print(TreeNodeOperator* node);
};

} // namespace bonk::ede_backend
