#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "protocol.h"
#include <lnos/known_node.h>

namespace lnos {

    struct Config {
        std::string name;
        std::vector<Service> services;
    };

    Config loadConfig();

    bool setConfig(const std::string& key, const std::string& value);

    bool createConfig();

    // Appends a node as: <name> <hex-encoded public key>.
    bool addKnownNode(const KnownNode& node);

    std::unordered_map<std::string, KnownNode> loadKnownNodes();
}
