
#include "x86_call.hpp"

namespace bonk::x86_backend {

CallCommand::CallCommand(CommandParameterSymbol symbol, const AbstractRegister* call_arguments,
                           int amount) {
    type = COMMAND_CALL;
    assert(symbol.ip_relative);

    parameters.resize(1);
    parameters[0] = CommandParameter::create_symbol(symbol);

    for (int i = 0; i < amount; i++) {
        set_read_register(call_arguments[i]);
    }
}

void CallCommand::to_bytes(CommandEncoder* buffer) {
    buffer->buffer.push_back(0xE8);
    int32_t current_offset = buffer->buffer.size();
    for (int i = 0; i < 4; i++)
        buffer->buffer.push_back(0x00);

    buffer->request_relocation(RelocationRequest{.relocation = parameters[0].symbol.symbol,
                                                  .address = current_offset,
                                                  .type = macho::RELOCATION_TYPE_CALL,
                                                  .data_length = 4,
                                                  .pc_rel = true});
}

} // namespace bonk::x86_backend