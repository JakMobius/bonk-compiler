
#pragma once

namespace bonk::x86_backend {

struct BackendContext;

}

#include "../../../utils/linear_allocator.hpp"
#include "../../compiler.hpp"
#include "../../target/macho/macho.hpp"
#include "colorizer/x86_colorizer.hpp"
#include "instructions/x86_add.hpp"
#include "instructions/x86_call.hpp"
#include "instructions/x86_colorizer_dead_end.hpp"
#include "instructions/x86_colorizer_frame.hpp"
#include "instructions/x86_colorizer_reg_preserve.hpp"
#include "instructions/x86_colorizer_repeat_scope.hpp"
#include "instructions/x86_colorizer_scope.hpp"
#include "instructions/x86_colorizer_scope_pop.hpp"
#include "instructions/x86_command.hpp"
#include "instructions/x86_mov.hpp"
#include "instructions/x86_nop.hpp"
#include "instructions/x86_pop.hpp"
#include "instructions/x86_push.hpp"
#include "instructions/x86_ret.hpp"
#include "instructions/x86_sub.hpp"
#include "instructions/x86_xor.hpp"
#include "x86_abstract_register.hpp"
#include "x86_backend_context_state.hpp"
#include "x86_field_list.hpp"
#include "x86_machine_register.hpp"
#include "x86_reg_stack.hpp"
#include "x86_register_descriptor_list.hpp"
#include "x86_register_extensions.hpp"
#include "x86_scope_stack.hpp"

namespace bonk::x86_backend {

extern const MachineRegister SYSTEM_V_ARGUMENT_REGISTERS[];
extern const int SYSTEM_V_ARGUMENT_REGISTERS_COUNT;
extern const MachineRegister SYSTEM_V_CALLEE_PRESERVED_REGISTERS[];
extern const int SYSTEM_V_CALLEE_PRESERVED_REGISTERS_COUNT;
extern const MachineRegister SYSTEM_V_CALLER_PRESERVED_REGISTERS[];
extern const int SYSTEM_V_CALLER_PRESERVED_REGISTERS_COUNT;

struct BackendContext {

    Compiler* linked_compiler = nullptr;

    BackendContext(Compiler* compiler, macho::MachoFile* target);

    ~BackendContext();

#if 0

    int procedure_parameters = 0;
    AbstractRegister procedure_return_register = 0;
    CommandBuffer* procedure_command_buffer = nullptr;
    std::vector<BackendContextState> state_stack = {};
    RegisterDescriptorList* current_descriptors = nullptr;
    RegisterDescriptorList* global_descriptors = nullptr;
    ScopeStack* scope_stack = nullptr;
    BackendContextState* state = nullptr;
    macho::MachoFile* target = nullptr;

    CommandList* procedure_body_container = nullptr;
    CommandList* cycle_container = nullptr;
    CommandList* cycle_body = nullptr;
    JmpLabel* cycle_head = nullptr;
    JmpLabel* cycle_tail = nullptr;

    void push_state();

    void pop_state();

    void compile_program(TreeNode* ast);

    FieldList* read_scope_variables(TreeNodeList* scope);

    void error_already_defined(TreeNodeIdentifier* identifier);

    void error_undefined_reference(TreeNodeIdentifier* node) const;

    Variable* field_list_declare_variable(TreeNodeVariableDefinition* node);

    void field_list_declare_block(TreeNodeBlockDefinition* node);

    FieldList* field_list_find_block_parameters(TreeNodeBlockDefinition* block) const;

    void write_block_promise(TreeNodeBlockDefinition* block);

    void write_block_implementation(TreeNodeBlockDefinition* block);

    void compile_block(TreeNodeList* block);

    void compile_line(TreeNode* node);

    void compile_plus(TreeNodeOperator* expression);

    void compile_minus(TreeNodeOperator* expression);

    void compile_multiply(TreeNodeOperator* expression);

    void compile_divide(TreeNodeOperator* expression);

    void compile_assignment(TreeNodeIdentifier* identifier, TreeNode* node);

    void compile_expression(TreeNode* node);

    AbstractRegister get_variable(TreeNodeIdentifier* identifier);

    void compile_cycle(TreeNodeCycle* cycle);

    void compile_block_definition(TreeNodeBlockDefinition* block);

    void push_initial_state();

    void pop_initial_state();

    void pop_to_scope(CommandList* scope) const;

    void compile_bonk_statement(TreeNodeOperator* pOperator);

    void compile_or(TreeNodeOperator* oper);

    void compile_and(TreeNodeOperator* oper);

    void compile_equals(TreeNodeOperator* oper);

    void compile_less_than(TreeNodeOperator* oper);

    void compile_less_or_equal_than(TreeNodeOperator* oper);

    void compile_greater_than(TreeNodeOperator* oper);

    void compile_greater_or_equal_than(TreeNodeOperator* oper);

    void compile_not_equal(TreeNodeOperator* oper);

    void compile_check(TreeNodeCheck* node);

    JumpCommand* append_oper_jmp_command(TreeNodeOperator* oper, bool inversed);

    void compile_brek_statement(TreeNodeOperator* oper);

    void compile_rebonk_statement(TreeNodeOperator* oper);

    bool can_use_fast_logic(TreeNodeOperator* oper);

    bool can_use_fast_logic_on_operand(TreeNode* operand, OperatorType oper_type);

    JumpCommand* compile_boolean_jump(TreeNode* node, bool check_for);

    JmpLabel* insert_label() const;

    JmpLabel* create_label() const;

    bool compile_logic_operand_recursive(TreeNode* node, JmpLabel* control_label,
                                         CommandList* control_label_list, OperatorType oper_type,
                                         bool check_for);

    bool compile_logic_recursive(TreeNodeOperator* oper, JmpLabel* control_label,
                                 CommandList* control_label_list, OperatorType oper_type,
                                 bool check_for);

    void compile_jump_logical(TreeNodeOperator* oper);

    void compile_call(TreeNodeCall* call);

    void preserve_callee_registers() const;

    void write_block_definition(TreeNodeBlockDefinition* definition) const;

    TreeNode*
    call_argument_list_get_value(TreeNodeList* argument_list,
                                 TreeNodeIdentifier* identifier);

    void locate_procedure_parameter(Variable* parameter);

    TreeNode* compile_nth_argument(VariableFunction* func,
                                   TreeNodeList* argument_list,
                                    int i);

    void write_global_var_definition(TreeNodeVariableDefinition* definition) const;
#endif
};

} // namespace bonk::x86_backend
