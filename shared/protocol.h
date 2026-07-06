#pragma once
#include <string>

namespace lnos {

    static constexpr const char* PREFIX = "HELLO";

    struct Packet {
        std::string name;
        std::string ip;
    };

    // сериализация
    inline std::string encode(const Packet& p) {
        return std::string(PREFIX) + "|" + p.name + "|" + p.ip;
    }

    // парсинг
    inline Packet decode(const std::string& msg) {
        Packet p;

        size_t p1 = msg.find('|');
        size_t p2 = msg.find('|', p1 + 1);

        if (p1 == std::string::npos || p2 == std::string::npos)
            return p;

        p.name = msg.substr(p1 + 1, p2 - p1 - 1);
        p.ip   = msg.substr(p2 + 1);

        return p;
    }

}