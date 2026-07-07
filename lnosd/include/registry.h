#pragma once

#include <string>
#include <unordered_map>
#include <chrono>

struct Node {
    std::string name;
    std::string ip;

    std::chrono::steady_clock::time_point lastSeen;
};

extern std::unordered_map<std::string, Node> nodes;