#pragma once
#include <string>

namespace lnos {

    struct Packet {
        std::string name;
    };

    inline std::string encode(const Packet& p) {
        return "ANNOUNCE|" + p.name;
    }

    inline Packet decode(const std::string& msg) {
        Packet p;

        size_t pos = msg.find('|');

        if (pos == std::string::npos)
            return p;

        p.name = msg.substr(pos + 1);

        return p;
    }

}