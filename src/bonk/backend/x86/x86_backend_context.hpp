
#pragma once

namespace bonk::x86_backend {

struct backend_context;

}

#include "../../../utils/linear_allocator.hpp"
#include "../../../utils/list.hpp"
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

extern const e_machine_register SYSTEM_V_ARGUMENT_REGISTERS[];
extern const int SYSTEM_V_ARGUMENT_REGISTERS_COUNT;
extern const e_machine_register SYSTEM_V_CALLEE_PRESERVED_REGISTERS[];
extern const int SYSTEM_V_CALLEE_PRESERVED_REGISTERS_COUNT;
extern const e_machine_register SYSTEM_V_CALLER_PRESERVED_REGISTERS[];
extern const int SYSTEM_V_CALLER_PRESERVED_REGISTERS_COUNT;

struct backend_context {

    int procedure_parameters = 0;
    abstract_register procedure_return_register = 0;
    command_buffer* procedure_command_buffer = nullptr;
    std::vector<backend_context_state> state_stack = {};
    register_descriptor_list* current_descriptors = nullptr;
    register_descriptor_list* global_descriptors = nullptr;
    scope_stack* scope_stack = nullptr;
    backend_context_state* state = nullptr;
    macho::macho_file* target = nullptr;

    command_list* procedure_body_container = nullptr;
    command_list* cycle_container = nullptr;
    command_list* cycle_body = nullptr;
    jmp_label* cycle_head = nullptr;
    jmp_label* cycle_tail = nullptr;

    compiler* linked_compiler = nullptr;

    backend_context(compiler* compiler, macho::macho_file* target);

    ~backend_context();

    void push_state();

    void pop_state();

    void compile_program(tree_node_list<tree_node*>* ast);

    field_list* read_scope_variables(tree_node_list<tree_node*>* scope);

    void error_already_defined(tree_node_identifier* identifier);

    void error_undefined_reference(tree_node_identifier* node);

    variable* field_list_declare_variable(tree_node_variable_definition* node);

    void field_list_declare_block(tree_node_block_definition* node);

    field_list* field_list_find_block_parameters(tree_node_block_definition* block);

    void write_block_promise(tree_node_block_definition* block);

    void write_block_implementation(tree_node_block_definition* block);

    void compile_block(tree_node_list<tree_node*>* block);

    void compile_line(tree_node* node);

    void compile_plus(tree_node_operator* expression);

    void compile_minus(tree_node_operator* expression);

    void compile_multiply(tree_node_operator* expression);

    void compile_divide(tree_node_operator* expression);

    void compile_assignment(tree_node_identifier* identifier, tree_node* node);

    void compile_expression(tree_node* node);

    abstract_register get_variable(tree_node_identifier* identifier);

    void compile_cycle(tree_node_cycle* cycle);

    void compile_block_definition(tree_node_block_definition* block);

    void push_initial_state();

    void pop_initial_state();

    void pop_to_scope(command_list* scope);

    void compile_bonk_statement(tree_node_operator* pOperator);

    void compile_or(tree_node_operator* oper);

    void compile_and(tree_node_operator* oper);

    void compile_equals(tree_node_operator* oper);

    void compile_less_than(tree_node_operator* oper);

    void compile_less_or_equal_than(tree_node_operator* oper);

    void compile_greater_than(tree_node_operator* oper);

    void compile_greater_or_equal_than(tree_node_operator* oper);

    void compile_not_equal(tree_node_operator* oper);

    void compile_check(tree_node_check* node);

    jump_command* append_oper_jmp_command(tree_node_operator* oper, bool inversed);

    void compile_brek_statement(tree_node_operator* oper);

    void compile_rebonk_statement(tree_node_operator* oper);

    bool can_use_fast_logic(tree_node_operator* oper);

    bool can_use_fast_logic_on_operand(tree_node* operand, operator_type oper_type);

    jump_command* compile_boolean_jump(tree_node* node, bool check_for);

    jmp_label* insert_label();

    jmp_label* create_label();

    bool compile_logic_operand_recursive(tree_node* node, jmp_label* control_label,
                                         command_list* control_label_list, operator_type oper_type,
                                         bool check_for);

    bool compile_logic_recursive(tree_node_operator* oper, jmp_label* control_label,
                                 command_list* control_label_list, operator_type oper_type,
                                 bool check_for);

    void compile_jump_logical(tree_node_operator* oper);

    void compile_call(tree_node_call* call);

    void preserve_callee_registers();

    void write_block_definition(tree_node_block_definition* definition);

    tree_node*
    call_argument_list_get_value(tree_node_list<tree_node_call_parameter*>* argument_list,
                                 tree_node_identifier* identifier);

    void locate_procedure_parameter(variable* parameter);

    tree_node* compile_nth_argument(variable_function* func,
                                    tree_node_list<tree_node_call_parameter*>* argument_list,
                                    int i);

    void write_global_var_definition(tree_node_variable_definition* definition);
};

} // namespace bonk::x86_backend
