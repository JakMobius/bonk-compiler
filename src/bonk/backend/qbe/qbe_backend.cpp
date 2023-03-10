
#include "qbe_backend.hpp"
#include "bonk/compiler.hpp"
#include "bonk/middleend/ir/hir.hpp"
#include "bonk/middleend/middleend.hpp"

void bonk::qbe_backend::QBEBackend::compile_program(bonk::IRProgram& program) {

    current_program = &program;

    for (auto& procedure : program.procedures) {
        compile_procedure(procedure);
    }
}

void bonk::qbe_backend::QBEBackend::compile_procedure(bonk::IRProcedure& procedure) {

    bool is_first = true;

    for (auto& block : procedure.base_blocks) {
        for (auto& instruction : block.instructions) {
            if (is_first) {
                if (!compile_procedure_header(*instruction)) {
                    // Procedure is external
                    return;
                }
                is_first = false;
            } else {
                compile_instruction(*instruction);
            }
        }
        assert(call_parameters.empty());
    }

    compile_procedure_footer();
}

bool bonk::qbe_backend::QBEBackend::compile_procedure_header(bonk::IRInstruction& instruction) {

    auto hir_instruction = static_cast<HIRInstruction&>(instruction);

    assert(hir_instruction.type == HIRInstructionType::procedure);

    auto& procedure = static_cast<HIRProcedure&>(instruction);

    if (procedure.is_external) {
        return false;
    }

    linked_compiler.config.output_file.get_stream()
        << "export function " << get_hir_type(procedure.return_type);
    linked_compiler.config.output_file.get_stream() << " ";
    TreeNode* procedure_definition = current_program->id_table.get_node(procedure.procedure_id);
    std::string_view procedure_name =
        current_program->symbol_table.symbol_names[procedure_definition];
    linked_compiler.config.output_file.get_stream() << "$\"_" << procedure_name << "\" (";

    for (int i = 0; i < procedure.parameters.size(); i++) {
        if (i != 0)
            linked_compiler.config.output_file.get_stream() << ", ";
        auto& parameter = procedure.parameters[i];
        linked_compiler.config.output_file.get_stream()
            << get_hir_type(parameter.type) << " %r" << parameter.register_id;
    }

    linked_compiler.config.output_file.get_stream() << ") {\n@start\n";

    return true;
}

void bonk::qbe_backend::QBEBackend::compile_procedure_footer() {
    linked_compiler.config.output_file.get_stream() << "}\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRLabel& instruction) {
    linked_compiler.config.output_file.get_stream() << "@L" << instruction.label_id << "\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRConstantLoad& instruction) {
    auto target = instruction.target;
    auto value = instruction.constant;
    auto type = instruction.type;

    padding();

    linked_compiler.config.output_file.get_stream()
        << "%r" << target << " =" << get_hir_type(type) << " copy ";

    switch (type) {
    case HIRDataType::byte:
    case HIRDataType::hword:
    case HIRDataType::word:
    case HIRDataType::dword:
        linked_compiler.config.output_file.get_stream() << value;
        break;
    case HIRDataType::float32:
        linked_compiler.config.output_file.get_stream()
            << "s_" << *reinterpret_cast<float*>(&value);
        break;
    case HIRDataType::float64:
        linked_compiler.config.output_file.get_stream()
            << "d_" << *reinterpret_cast<double*>(&value);
        break;
    default:
        assert(false);
    }

    linked_compiler.config.output_file.get_stream() << "\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRSymbolLoad& instruction) {
    auto type = instruction.type;

    padding();

    linked_compiler.config.output_file.get_stream()
        << "%r" << instruction.target << " =" << get_hir_type(type);

    TreeNode* symbol_definition = current_program->id_table.get_node(instruction.symbol_id);
    std::string_view symbol_name = current_program->symbol_table.symbol_names[symbol_definition];

    linked_compiler.config.output_file.get_stream() << " $\"_" << symbol_name << "\"\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIROperation& instruction) {
    auto target = instruction.target;

    padding();

    linked_compiler.config.output_file.get_stream()
        << "%r" << target << " =" << get_hir_type(instruction.result_type) << " ";

    switch (instruction.operation_type) {
    case HIROperationType::plus:
        linked_compiler.config.output_file.get_stream() << "add ";
        break;
    case HIROperationType::minus:
        linked_compiler.config.output_file.get_stream() << "sub ";
        break;
    case HIROperationType::multiply:
        linked_compiler.config.output_file.get_stream() << "mul ";
        break;
    case HIROperationType::divide:
        linked_compiler.config.output_file.get_stream() << "div ";
        break;
    case HIROperationType::assign:
        linked_compiler.config.output_file.get_stream() << "copy ";
        break;
    case HIROperationType::equal:
    case HIROperationType::not_equal:
    case HIROperationType::less:
    case HIROperationType::less_equal:
    case HIROperationType::greater:
    case HIROperationType::greater_equal:
        print_comparison(instruction.operation_type, instruction.operand_type);
        break;
    case HIROperationType::and_op:
    case HIROperationType::or_op:
    case HIROperationType::xor_op:
    case HIROperationType::not_op:
    default:
        assert(!"Cannot compile this just yet");
        break;
    }

    linked_compiler.config.output_file.get_stream() << "%r" << instruction.left;

    if (instruction.right.has_value()) {
        linked_compiler.config.output_file.get_stream() << ", %r" << instruction.right.value();
    }

    linked_compiler.config.output_file.get_stream() << "\n";
}

void bonk::qbe_backend::QBEBackend::print_comparison(HIROperationType type, HIRDataType operand_type) {

    linked_compiler.config.output_file.get_stream() << 'c';

    switch(operand_type) {
        case HIRDataType::byte:
        case HIRDataType::hword:
        case HIRDataType::word:
        case HIRDataType::dword:
            linked_compiler.config.output_file.get_stream() << 's';
            break;
        default:
            break;
    }

    switch (type) {
    case HIROperationType::equal:
        linked_compiler.config.output_file.get_stream() << "eq";
        break;
    case HIROperationType::not_equal:
        linked_compiler.config.output_file.get_stream() << "ne";
        break;
    case HIROperationType::less:
        linked_compiler.config.output_file.get_stream() << "lt";
        break;
    case HIROperationType::less_equal:
        linked_compiler.config.output_file.get_stream() << "le";
        break;
    case HIROperationType::greater:
        linked_compiler.config.output_file.get_stream() << "gt";
        break;
    case HIROperationType::greater_equal:
        linked_compiler.config.output_file.get_stream() << "ge";
        break;
    default:
        assert(false);
    }

    linked_compiler.config.output_file.get_stream() << get_hir_type(operand_type) << " ";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRJump& instruction) {
    padding();
    linked_compiler.config.output_file.get_stream() << "jmp @L" << instruction.label_id << "\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRJumpNZ& instruction) {
    padding();
    linked_compiler.config.output_file.get_stream()
        << "jnz %r" << instruction.condition << ", @L" << instruction.nz_label << ", @L"
        << instruction.z_label << "\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRCall& instruction) {
    padding();

    if (instruction.return_value.has_value()) {
        linked_compiler.config.output_file.get_stream()
            << "%r" << instruction.return_value.value() << " =";

        linked_compiler.config.output_file.get_stream()
            << get_hir_type(instruction.return_type) << " ";
    }

    TreeNode* symbol_definition =
        current_program->id_table.get_node(instruction.procedure_label_id);
    std::string_view symbol_name = current_program->symbol_table.symbol_names[symbol_definition];

    linked_compiler.config.output_file.get_stream() << "call $\"_" << symbol_name << "\"(";

    for (int i = 0; i < call_parameters.size(); i++) {
        if (i != 0)
            linked_compiler.config.output_file.get_stream() << ", ";
        linked_compiler.config.output_file.get_stream()
            << get_hir_type(call_parameters[i].type) << " %r" << call_parameters[i].register_id;
    }
    call_parameters.clear();

    linked_compiler.config.output_file.get_stream() << ")\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRReturn& instruction) {
    padding();
    linked_compiler.config.output_file.get_stream() << "ret";
    if (instruction.return_value.has_value()) {
        linked_compiler.config.output_file.get_stream()
            << " %r" << instruction.return_value.value();
    }
    linked_compiler.config.output_file.get_stream() << "\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRParameter& instruction) {
    call_parameters.push_back({instruction.type, instruction.parameter});
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRMemoryLoad& instruction) {
    padding();
    linked_compiler.config.output_file.get_stream()
        << "%r" << instruction.target << " =" << get_hir_type(instruction.type) << " load"
        << get_hir_type(instruction.type, false);

    if(instruction.type == HIRDataType::hword || instruction.type == HIRDataType::byte) {
        linked_compiler.config.output_file.get_stream() << "s";
    }

    linked_compiler.config.output_file.get_stream() << "%r" << instruction.address << "\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(HIRMemoryStore& instruction) {
    padding();
    linked_compiler.config.output_file.get_stream()
        << "store" << get_hir_type(instruction.type, false) << " %r" << instruction.value << ", %r"
        << instruction.address << "\n";
}

void bonk::qbe_backend::QBEBackend::compile_instruction(bonk::IRInstruction& instruction) {

    auto hir_instruction = static_cast<HIRInstruction&>(instruction);

    switch (hir_instruction.type) {
    case HIRInstructionType::label:
        return compile_instruction(static_cast<HIRLabel&>(instruction));
    case HIRInstructionType::constant_load:
        return compile_instruction(static_cast<HIRConstantLoad&>(instruction));
    case HIRInstructionType::symbol_load:
        return compile_instruction(static_cast<HIRSymbolLoad&>(instruction));
    case HIRInstructionType::operation:
        return compile_instruction(static_cast<HIROperation&>(instruction));
    case HIRInstructionType::jump:
        return compile_instruction(static_cast<HIRJump&>(instruction));
    case HIRInstructionType::jump_nz:
        return compile_instruction(static_cast<HIRJumpNZ&>(instruction));
    case HIRInstructionType::call:
        return compile_instruction(static_cast<HIRCall&>(instruction));
    case HIRInstructionType::return_op:
        return compile_instruction(static_cast<HIRReturn&>(instruction));
    case HIRInstructionType::parameter:
        return compile_instruction(static_cast<HIRParameter&>(instruction));
    case HIRInstructionType::memory_load:
        return compile_instruction(static_cast<HIRMemoryLoad&>(instruction));
    case HIRInstructionType::memory_store:
        return compile_instruction(static_cast<HIRMemoryStore&>(instruction));
    case HIRInstructionType::procedure:
        assert(!"Procedure header occurred in the middle of the procedure");
    default:
        assert(!"Unknown instruction type");
    }
}

char bonk::qbe_backend::QBEBackend::get_hir_type(bonk::HIRDataType type, bool base_type) {
    switch (type) {
    case HIRDataType::float32:
        return 's';
    case HIRDataType::float64:
        return 'd';
    case HIRDataType::word:
        return 'w';
    case HIRDataType::dword:
        return 'l';
    case HIRDataType::byte:
        return base_type ? 'w' : 'b';
    case HIRDataType::hword:
        return base_type ? 'w' : 'h';
    default:
        assert(false);
    }
}

void bonk::qbe_backend::QBEBackend::padding() {
    linked_compiler.config.output_file.get_stream() << "    ";
}
