
#include "hir_printer.hpp"

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRConstantLoad& instruction) const {

    stream.get_stream() << "%" << instruction.target << " <- ";

    print(instruction.type);

    stream.get_stream() << " ";

    switch (instruction.type) {
    case HIRDataType::byte:
        stream.get_stream() << static_cast<uint8_t>(instruction.constant);
        break;
    case HIRDataType::hword:
        stream.get_stream() << static_cast<uint16_t>(instruction.constant);
        break;
    case HIRDataType::word:
        stream.get_stream() << static_cast<uint32_t>(instruction.constant);
        break;
    case HIRDataType::dword:
        stream.get_stream() << static_cast<uint64_t>(instruction.constant);
        break;
    case HIRDataType::float32:
        stream.get_stream() << *reinterpret_cast<const float*>(&instruction.constant);
        break;
    case HIRDataType::float64:
        stream.get_stream() << *reinterpret_cast<const double*>(&instruction.constant);
        break;
    default:
        assert(false);
        break;
    }
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRLabel& instruction) const {
    print_label(program, instruction.label_id);
    stream.get_stream() << ":";
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRSymbolLoad& instruction) const {
    stream.get_stream() << "%" << instruction.target << " <- symbol " << instruction.symbol_id;
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRJump& instruction) const {
    stream.get_stream() << "jmp ";
    print_label(program, instruction.label_id);
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRJumpNZ& instruction) const {
    stream.get_stream() << "jmpnz %" << instruction.condition << ", ";
    print_label(program, instruction.nz_label);
    stream.get_stream() << ", ";
    print_label(program, instruction.z_label);
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program,
                             const bonk::HIRMemoryLoad& instruction) const {
    stream.get_stream() << "%" << instruction.target << " <- load ";
    print(instruction.type);
    stream.get_stream() << " %" << instruction.address;
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program,
                             const bonk::HIRMemoryStore& instruction) const {
    stream.get_stream() << "store ";
    print(instruction.type);
    stream.get_stream() << " %" << instruction.value << ", %" << instruction.address;
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRCall& instruction) const {

    if (instruction.return_value.has_value()) {
        stream.get_stream() << "%" << instruction.return_value.value() << " <- ";
    }

    stream.get_stream() << "call ";

    print_label(program, instruction.procedure_label_id);
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRReturn& instruction) const {
    stream.get_stream() << "ret";
    if (instruction.return_value.has_value()) {
        stream.get_stream() << " ";
        print(instruction.return_type);
        stream.get_stream() << " %" << instruction.return_value.value();
    }
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIROperation& instruction) const {

    stream.get_stream() << "%" << instruction.target << " <- ";
    print(instruction.operand_type);
    stream.get_stream() << " ";
    print(instruction.operation_type);
    stream.get_stream() << " %" << instruction.left;

    if (instruction.right.has_value()) {
        stream.get_stream() << ", %" << instruction.right.value();
    }
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRProcedure& instruction) const {
    if(instruction.is_external) {
        stream.get_stream() << "external ";
    }
    stream.get_stream() << "blok ";

    auto node = program.id_table.get_node(instruction.procedure_id);
    stream.get_stream() << program.symbol_table.symbol_names[node];

    stream.get_stream() << ": (";
    for (int i = 0; i < instruction.parameters.size(); i++) {
        stream.get_stream() << "%" << instruction.parameters[i].register_id << ": ";

        print(instruction.parameters[i].type);

        if (i != instruction.parameters.size() - 1) {
            stream.get_stream() << ", ";
        }
    }
    stream.get_stream() << ") -> ";
    print(instruction.return_type);
    stream.get_stream() << "\n";
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRParameter& instruction) const {
    stream.get_stream() << "param ";
    print(instruction.type);
    stream.get_stream() << " %" << instruction.parameter;
}

void bonk::HIRPrinter::print(bonk::HIROperationType type) const {
    switch (type) {
    case HIROperationType::plus:
        stream.get_stream() << "+";
        break;
    case HIROperationType::minus:
        stream.get_stream() << "-";
        break;
    case HIROperationType::multiply:
        stream.get_stream() << "*";
        break;
    case HIROperationType::divide:
        stream.get_stream() << "/";
        break;
    case HIROperationType::assign:
        stream.get_stream() << "=";
        break;
    case HIROperationType::and_op:
        stream.get_stream() << "&";
        break;
    case HIROperationType::or_op:
        stream.get_stream() << "|";
        break;
    case HIROperationType::xor_op:
        stream.get_stream() << "^";
        break;
    case HIROperationType::not_op:
        stream.get_stream() << "~";
        break;
    case HIROperationType::equal:
        stream.get_stream() << "==";
        break;
    case HIROperationType::not_equal:
        stream.get_stream() << "!=";
        break;
    case HIROperationType::less:
        stream.get_stream() << "<";
        break;
    case HIROperationType::less_equal:
        stream.get_stream() << "<=";
        break;
    case HIROperationType::greater:
        stream.get_stream() << ">";
        break;
    case HIROperationType::greater_equal:
        stream.get_stream() << ">=";
        break;
    }
}

void bonk::HIRPrinter::print(bonk::HIRDataType type) const {
    switch (type) {
    case HIRDataType::byte:
        stream.get_stream() << "byte";
        break;
    case HIRDataType::hword:
        stream.get_stream() << "hword";
        break;
    case HIRDataType::word:
        stream.get_stream() << "word";
        break;
    case HIRDataType::dword:
        stream.get_stream() << "dword";
        break;
    case HIRDataType::float32:
        stream.get_stream() << "float32";
        break;
    case HIRDataType::float64:
        stream.get_stream() << "float64";
        break;
    case HIRDataType::unset:
        stream.get_stream() << "unset";
        break;
    }
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program, const bonk::HIRInstruction& instruction) const {
    switch (instruction.type) {
    case HIRInstructionType::constant_load:
        print(program, static_cast<const HIRConstantLoad&>(instruction));
        break;
    case HIRInstructionType::symbol_load:
        print(program, static_cast<const HIRSymbolLoad&>(instruction));
        break;
    case HIRInstructionType::jump:
        print(program, static_cast<const HIRJump&>(instruction));
        break;
    case HIRInstructionType::jump_nz:
        print(program, static_cast<const HIRJumpNZ&>(instruction));
        break;
    case HIRInstructionType::call:
        print(program, static_cast<const HIRCall&>(instruction));
        break;
    case HIRInstructionType::return_op:
        print(program, static_cast<const HIRReturn&>(instruction));
        break;
    case HIRInstructionType::operation:
        print(program, static_cast<const HIROperation&>(instruction));
        break;
    case HIRInstructionType::procedure:
        print(program, static_cast<const HIRProcedure&>(instruction));
        break;
    case HIRInstructionType::label:
        print(program, static_cast<const HIRLabel&>(instruction));
        break;
    case HIRInstructionType::parameter:
        print(program, static_cast<const HIRParameter&>(instruction));
        break;
    case HIRInstructionType::memory_load:
        print(program, static_cast<const HIRMemoryLoad&>(instruction));
        break;
    case HIRInstructionType::memory_store:
        print(program, static_cast<const HIRMemoryStore&>(instruction));
        break;
    default:
        assert(!"Unknown instruction type");
    }
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program) const {
    for (const auto& procedure : program.procedures) {
        stream.get_stream() << "procedure {\n";
        for (const auto& base_block : procedure.base_blocks) {
            for (const auto& instruction : base_block.instructions) {
                stream.get_stream() << "    ";
                print(program, static_cast<const bonk::HIRInstruction&>(*instruction));
                stream.get_stream() << "\n";
            }
        }
        stream.get_stream() << "}\n";
    }
}
void bonk::HIRPrinter::print_label(const bonk::IRProgram& program, int label) const {

    auto node = program.id_table.get_node(label);
    auto it =  program.symbol_table.symbol_names.find(node);

    if(it != program.symbol_table.symbol_names.end()) {
        stream.get_stream() << it->second;
    } else {
        stream.get_stream() << "L" << label;
    }

}
