
#include "x86_call.hpp"

namespace bonk::x86_backend {

call_command::call_command(command_parameter_symbol symbol, abstract_register* call_arguments,
                           int amount) {
    type = COMMAND_CALL;
    assert(symbol.ip_relative);

    parameters.resize(1);
    parameters[0] = command_parameter::create_symbol(symbol);

    for (int i = 0; i < amount; i++) {
        set_read_register(call_arguments[i]);
    }
}

void call_command::to_bytes(command_encoder* buffer) {
    buffer->buffer.push_back(0xE8);
    int32_t current_offset = buffer->buffer.size();
    for (int i = 0; i < 4; i++)
        buffer->buffer.push_back(0x00);

    buffer->request_relocation(relocation_request{.relocation = parameters[0].symbol.symbol,
                                                  .address = current_offset,
                                                  .type = macho::RELOCATION_TYPE_CALL,
                                                  .data_length = 4,
                                                  .pc_rel = true});
}

} // namespace bonk::x86_backend