#include "registry.h"

#include <string>

std::unordered_map<std::string, Node> nodes;
std::unordered_map<std::string, lnos::KnownNode> knownNodes;