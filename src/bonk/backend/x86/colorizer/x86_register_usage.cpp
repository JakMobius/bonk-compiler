
#include "x86_register_usage.hpp"

namespace bonk::x86_backend {

bool AbstractRegisterUsage::operator<(const AbstractRegisterUsage& right) const {
    if (instruction_index < right.instruction_index)
        return true;
    if (instruction_index > right.instruction_index)
        return false;
    if (!is_write && right.is_write)
        return true;
    return false;
}

} // namespace bonk::x86_backend