
#pragma once

namespace bonk::x86_backend {

struct RegisterExtensions;

}

#include <cstdlib>

namespace bonk::x86_backend {

struct RegisterExtensions {
    bool w;
    bool r;
    bool x;
    bool b;

    unsigned char get_byte() const;

    bool exist() const;
};

} // namespace bonk::x86_backend
