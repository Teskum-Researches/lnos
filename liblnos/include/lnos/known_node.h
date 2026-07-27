#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <lnos/protocol.h>

namespace lnos {
    struct KnownNode {
        std::string name;
        std::array<std::uint8_t, PUBLIC_KEY_SIZE> publicKey;
    };
}