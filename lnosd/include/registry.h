#pragma once

#include <string>
#include <unordered_map>
#include <chrono>

#include "lnos/protocol.h"
#include <lnos/known_node.h>

enum class NodeStatus {
    Online,
    Offline
};

struct Node {
    std::string name;
    std::string ip;

    std::vector<lnos::Service> services;

    std::chrono::steady_clock::time_point lastSeen;
    NodeStatus status;
};

extern std::unordered_map<std::string, Node> nodes;
extern std::unordered_map<std::string, lnos::KnownNode> knownNodes;
