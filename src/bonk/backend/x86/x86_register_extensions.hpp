
#pragma once

namespace bonk::x86_backend {

struct register_extensions;

}

#include <cstdlib>

namespace bonk::x86_backend {

struct register_extensions {
    bool w;
    bool r;
    bool x;
    bool b;

    unsigned char get_byte() const;

    bool exist() const;
};

} // namespace bonk::x86_backend
