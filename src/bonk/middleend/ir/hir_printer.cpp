
#include "hir_printer.hpp"

void bonk::HIRPrinter::print(const bonk::HIRLabel& instruction) const {
    stream.get_stream() << "L" << instruction.label_id << ":";
}

void bonk::HIRPrinter::print(const bonk::HIRConstantLoad& instruction) const {

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
    }
}

void bonk::HIRPrinter::print(const bonk::HIRSymbolLoad& instruction) const {
    stream.get_stream() << "%" << instruction.target << " <- symbol " << instruction.symbol_id;
}

void bonk::HIRPrinter::print(const bonk::HIRJump& instruction) const {
    stream.get_stream() << "jmp L" << instruction.label_id;
}

void bonk::HIRPrinter::print(const bonk::HIRJumpNZ& instruction) const {
    stream.get_stream() << "jmpnz %" << instruction.condition << ", L" << instruction.nz_label
                        << ", L" << instruction.z_label;
}

void bonk::HIRPrinter::print(const bonk::HIRCall& instruction) const {

    if (instruction.return_value.has_value()) {
        stream.get_stream() << "%" << instruction.return_value.value() << " <- ";
    }

    stream.get_stream() << "call ";

    if (middle_end) {
        auto node = middle_end->id_table.get_node(instruction.procedure_label_id);
        stream.get_stream() << middle_end->symbol_table.symbol_names[node];
    } else {
        stream.get_stream() << "L" << instruction.procedure_label_id;
    }
}

void bonk::HIRPrinter::print(const bonk::HIRReturn& instruction) const {
    stream.get_stream() << "ret";
    if (instruction.return_value.has_value()) {
        stream.get_stream() << " ";
        print(instruction.return_type);
        stream.get_stream() << " %" << instruction.return_value.value();
    }
}

void bonk::HIRPrinter::print(const bonk::HIROperation& instruction) const {

    stream.get_stream() << "%" << instruction.target << " <- ";
    print(instruction.operand_type);
    stream.get_stream() << " ";
    print(instruction.operation_type);
    stream.get_stream() << " %" << instruction.left;

    if (instruction.right.has_value()) {
        stream.get_stream() << ", %" << instruction.right.value();
    }
}

void bonk::HIRPrinter::print(const bonk::HIRProcedure& instruction) const {
    stream.get_stream() << "blok ";

    if (middle_end) {
        auto node = middle_end->id_table.get_node(instruction.procedure_id);
        stream.get_stream() << middle_end->symbol_table.symbol_names[node];
    } else {
        stream.get_stream() << "L" << instruction.procedure_id;
    }

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

void bonk::HIRPrinter::print(const bonk::HIRParameter& instruction) const {
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
    }
}

void bonk::HIRPrinter::print(const bonk::HIRInstruction& instruction) const {
    switch (instruction.type) {
    case HIRInstructionType::constant_load:
        print(static_cast<const HIRConstantLoad&>(instruction));
        break;
    case HIRInstructionType::symbol_load:
        print(static_cast<const HIRSymbolLoad&>(instruction));
        break;
    case HIRInstructionType::jump:
        print(static_cast<const HIRJump&>(instruction));
        break;
    case HIRInstructionType::jump_nz:
        print(static_cast<const HIRJumpNZ&>(instruction));
        break;
    case HIRInstructionType::call:
        print(static_cast<const HIRCall&>(instruction));
        break;
    case HIRInstructionType::return_op:
        print(static_cast<const HIRReturn&>(instruction));
        break;
    case HIRInstructionType::operation:
        print(static_cast<const HIROperation&>(instruction));
        break;
    case HIRInstructionType::procedure:
        print(static_cast<const HIRProcedure&>(instruction));
        break;
    case HIRInstructionType::label:
        print(static_cast<const HIRLabel&>(instruction));
        break;
    case HIRInstructionType::parameter:
        print(static_cast<const HIRParameter&>(instruction));
        break;
    }
}

void bonk::HIRPrinter::print(const bonk::IRProgram& program) const {
    for (const auto& procedure : program.procedures) {
        stream.get_stream() << "procedure {\n";
        for (const auto& base_block : procedure.base_blocks) {
            for (const auto& instruction : base_block.instructions) {
                stream.get_stream() << "    ";
                print(static_cast<const bonk::HIRInstruction&>(*instruction));
                stream.get_stream() << "\n";
            }
        }
        stream.get_stream() << "}\n";
    }
}
