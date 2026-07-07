#pragma once

#include <string>

namespace lnos {

    struct Config {
        std::string nodeName;
        std::string interface;
    };

    Config loadConfig();
    void createConfig();

}