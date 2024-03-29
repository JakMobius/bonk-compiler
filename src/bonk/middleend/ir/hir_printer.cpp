
#include "hir_printer.hpp"

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRConstantLoadInstruction& instruction) const {
    padding();
    stream.get_stream() << "%" << instruction.target << " <- ";

    print(instruction.type);

    stream.get_stream() << " ";

    switch (instruction.type) {
    case HIRDataType::byte:
        stream.get_stream() << static_cast<int8_t>(instruction.constant);
        break;
    case HIRDataType::hword:
        stream.get_stream() << static_cast<int16_t>(instruction.constant);
        break;
    case HIRDataType::word:
        stream.get_stream() << static_cast<int32_t>(instruction.constant);
        break;
    case HIRDataType::dword:
        stream.get_stream() << static_cast<int64_t>(instruction.constant);
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

    stream.get_stream() << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRLabelInstruction& instruction) const {
    print_label(block, instruction.label_id);
    stream.get_stream() << ":\n";
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRSymbolLoadInstruction& instruction) const {
    padding();
    stream.get_stream() << "%" << instruction.target << " <- symbol " << instruction.symbol_id
                        << "\n";
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRJumpInstruction& instruction) const {
    padding();
    stream.get_stream() << "jmp L" << instruction.label_id << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRJumpNZInstruction& instruction) const {
    padding();
    stream.get_stream() << "jmpnz %" << instruction.condition << ", L" << instruction.nz_label
                        << ", L" << instruction.z_label << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRMemoryLoadInstruction& instruction) const {
    padding();
    stream.get_stream() << "%" << instruction.target << " <- load ";
    print(instruction.type);
    stream.get_stream() << " %" << instruction.address;
    stream.get_stream() << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRMemoryStoreInstruction& instruction) const {
    padding();
    stream.get_stream() << "store ";
    print(instruction.type);
    stream.get_stream() << " %" << instruction.value << ", %" << instruction.address;
    stream.get_stream() << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRCallInstruction& instruction) const {
    padding();

    if (instruction.return_value.has_value()) {
        stream.get_stream() << "%" << instruction.return_value.value() << " <- ";
    }

    stream.get_stream() << "call ";

    print_label(block, instruction.procedure_label_id);

    stream.get_stream() << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRReturnInstruction& instruction) const {
    padding();

    stream.get_stream() << "ret";
    if (instruction.return_value.has_value()) {
        stream.get_stream() << " ";
        print(instruction.return_type);
        stream.get_stream() << " %" << instruction.return_value.value();
    }

    stream.get_stream() << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIROperationInstruction& instruction) const {
    padding();

    stream.get_stream() << "%" << instruction.target << " <- ";
    print(instruction.operand_type);
    stream.get_stream() << " ";
    print(instruction.operation_type);
    stream.get_stream() << " %" << instruction.left;

    if (instruction.right.has_value()) {
        stream.get_stream() << ", %" << instruction.right.value();
    }

    stream.get_stream() << '\n';
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRParameterInstruction& instruction) const {
    padding();
    stream.get_stream() << "param ";
    print(instruction.type);
    stream.get_stream() << " %" << instruction.parameter;
    stream.get_stream() << '\n';
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

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRIncRefCounterInstruction& instruction) const {
    padding();
    stream.get_stream() << "inc_ref %" << instruction.address << "\n";
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRDecRefCounterInstruction& instruction) const {
    padding();
    stream.get_stream() << "dec_ref %" << instruction.address << " (hive "
                        << instruction.hive_definition->hive_name->identifier_text << ")\n";
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRPhiFunctionInstruction& instruction) const {
    padding();
    assert(instruction.sources.size() == block.predecessors.size());
    stream.get_stream() << "%" << instruction.target << " <- phi (";
    for (int i = 0; i < instruction.sources.size(); i++) {
        if (i != 0)
            stream.get_stream() << ", ";
        stream.get_stream() << "L" << block.predecessors[i]->index << " = %"
                                << instruction.sources[i];
    }
    stream.get_stream() << ")\n";
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRInstruction& instruction) const {
    switch (instruction.type) {
    case HIRInstructionType::constant_load:
        print(block, static_cast<const HIRConstantLoadInstruction&>(instruction));
        break;
    case HIRInstructionType::symbol_load:
        print(block, static_cast<const HIRSymbolLoadInstruction&>(instruction));
        break;
    case HIRInstructionType::jump:
        print(block, static_cast<const HIRJumpInstruction&>(instruction));
        break;
    case HIRInstructionType::jump_nz:
        print(block, static_cast<const HIRJumpNZInstruction&>(instruction));
        break;
    case HIRInstructionType::call:
        print(block, static_cast<const HIRCallInstruction&>(instruction));
        break;
    case HIRInstructionType::return_op:
        print(block, static_cast<const HIRReturnInstruction&>(instruction));
        break;
    case HIRInstructionType::operation:
        print(block, static_cast<const HIROperationInstruction&>(instruction));
        break;
    case HIRInstructionType::label:
        print(block, static_cast<const HIRLabelInstruction&>(instruction));
        break;
    case HIRInstructionType::parameter:
        print(block, static_cast<const HIRParameterInstruction&>(instruction));
        break;
    case HIRInstructionType::memory_load:
        print(block, static_cast<const HIRMemoryLoadInstruction&>(instruction));
        break;
    case HIRInstructionType::memory_store:
        print(block, static_cast<const HIRMemoryStoreInstruction&>(instruction));
        break;
    case HIRInstructionType::inc_ref_counter:
        print(block, static_cast<const HIRIncRefCounterInstruction&>(instruction));
        break;
    case HIRInstructionType::dec_ref_counter:
        print(block, static_cast<const HIRDecRefCounterInstruction&>(instruction));
        break;
    case HIRInstructionType::file:
        print(block, static_cast<const HIRFileInstruction&>(instruction));
        break;
    case HIRInstructionType::location:
        print(block, static_cast<const HIRLocationInstruction&>(instruction));
        break;
    case HIRInstructionType::phi_function:
        print(block, static_cast<const HIRPhiFunctionInstruction&>(instruction));
        break;
    default:
        assert(!"Unknown instruction type");
    }
}

void bonk::HIRPrinter::print(const bonk::HIRProgram& program) const {
    for (const auto& procedure : program.procedures) {
        print(*procedure);
    }
}

void bonk::HIRPrinter::print(const bonk::HIRProcedure& procedure) const {

    if (procedure.is_external) {
        stream.get_stream() << "external ";
    }
    stream.get_stream() << "procedure ";

    auto node = procedure.program.id_table.get_node(procedure.procedure_id);
    stream.get_stream() << procedure.program.symbol_table.symbol_names[node];

    stream.get_stream() << ": (";
    for (int i = 0; i < procedure.parameters.size(); i++) {
        stream.get_stream() << "%" << procedure.parameters[i].register_id << ": ";

        print(procedure.parameters[i].type);

        if (i != procedure.parameters.size() - 1) {
            stream.get_stream() << ", ";
        }
    }
    stream.get_stream() << ") -> ";
    print(procedure.return_type);

    if (procedure.is_external) {
        stream.get_stream() << ";";
        return;
    }

    stream.get_stream() << " {\n";

    for (const auto& base_block : procedure.base_blocks) {
        stream.get_stream() << "L" << base_block->index << ":\n";
        for (const auto& instruction : base_block->instructions) {
            print(*base_block, static_cast<const bonk::HIRInstruction&>(*instruction));
        }
    }
    stream.get_stream() << "}\n";
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRFileInstruction& instruction) const {
    padding();
    stream.get_stream() << "file " << instruction.file << "\n";
}

void bonk::HIRPrinter::print(const bonk::HIRBaseBlock& block,
                             const bonk::HIRLocationInstruction& instruction) const {
    padding();
    stream.get_stream() << "loc " << instruction.line << ":" << instruction.column << "\n";
}

void bonk::HIRPrinter::print_label(const bonk::HIRBaseBlock& block, int label) const {

    auto& program = block.procedure.program;
    auto node = program.id_table.get_node(label);

    if (node) {
        auto it = program.symbol_table.symbol_names.find(node);
        if (it != program.symbol_table.symbol_names.end()) {
            stream.get_stream() << it->second;
            return;
        }
    }

    stream.get_stream() << "L" << label;
}

void bonk::HIRPrinter::padding() const {
    stream.get_stream() << "    ";
}
