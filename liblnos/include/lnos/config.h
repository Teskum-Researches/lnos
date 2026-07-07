#pragma once

#include <string>

namespace lnos {

    struct Config {
        std::string nodeName;
        std::string interface;
    };

    Config loadConfig();

    bool setConfig(const std::string& key, const std::string& value);

    bool createConfig();

}