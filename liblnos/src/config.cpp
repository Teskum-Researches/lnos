#include <fstream>
#include <sstream>
#include <lnos/config.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unistd.h>

#include <lnos/crypto.h>

namespace lnos {

    namespace {
        constexpr char hexDigits[] = "0123456789abcdef";

        std::string publicKeyToHex(const std::array<std::uint8_t, PUBLIC_KEY_SIZE>& key)
        {
            std::string result;
            result.reserve(key.size() * 2);

            for (const auto byte : key) {
                result += hexDigits[byte >> 4];
                result += hexDigits[byte & 0x0f];
            }

            return result;
        }

        int hexValue(char character)
        {
            if (character >= '0' && character <= '9')
                return character - '0';
            if (character >= 'a' && character <= 'f')
                return character - 'a' + 10;
            if (character >= 'A' && character <= 'F')
                return character - 'A' + 10;
            return -1;
        }

        bool hexToPublicKey(const std::string& hex,
                            std::array<std::uint8_t, PUBLIC_KEY_SIZE>& key)
        {
            if (hex.size() != key.size() * 2)
                return false;

            for (std::size_t i = 0; i < key.size(); ++i) {
                const int high = hexValue(hex[i * 2]);
                const int low = hexValue(hex[i * 2 + 1]);
                if (high < 0 || low < 0)
                    return false;

                key[i] = static_cast<std::uint8_t>((high << 4) | low);
            }

            return true;
        }
    }

    Config loadConfig()
    {
        Config cfg;

        std::ifstream nameFile("/etc/lnos/name");

        if (nameFile.is_open()) {
            nameFile >> cfg.name;
        } else {
            char host[256];
            gethostname(host, sizeof(host));
            auto hostname = std::string(host);
            cfg.name = hostname;
        }

        std::ifstream servicesFile("/etc/lnos/services");

        if (servicesFile.is_open()) {
            std::string line;
            while (std::getline(servicesFile, line)) {
                std::stringstream ss(line);

                Service service;

                ss >> service.name;
                ss >> service.port;

                cfg.services.push_back(service);
            }
        }

        return cfg;
    }


    bool setConfig(const std::string& key, const std::string& value)
    {
        if (geteuid() != 0)
            return false;

        createConfig();

        if (key == "name") {

            std::ofstream file("/etc/lnos/name");

            if (file.is_open()) {
                file << value << std::endl;
                return true;
            }

            return false;
        }

        return false;
    }

    std::string getConfig(const std::string& key)
    {
        if (geteuid() != 0) {
            return "";
        }

        if (key == "name") {

            std::ifstream file("/etc/lnos/name");

            if (file.is_open()) {
                std::string value;
                file >> value;
                return value;
            }

            return "";
        }

        return "";
    }


    bool createConfig()
    {
        if (access("/etc/lnos", F_OK) != 0)
        {
            if (mkdir("/etc/lnos", 0755) != 0)
                return false;
            chmod("/etc/lnos", 0755);
        }

        if (access("/etc/lnos/name", F_OK) != 0)
        {
            std::ofstream nameFile("/etc/lnos/name");

            if (!nameFile.is_open())
                return false;

            nameFile << "default.node\n";
            chmod("/etc/lnos/name", 0644);
        }

        if (access("/etc/lnos/services", F_OK) != 0)
        {
            std::ofstream servicesFile("/etc/lnos/services");

            if (!servicesFile.is_open())
                return false;

            chmod("/etc/lnos/services", 0644);
        }

        if (access("/etc/lnos/known_nodes", F_OK) != 0)
        {
            std::ofstream knownNodesFile("/etc/lnos/known_nodes");

            if (!knownNodesFile.is_open())
                return false;

            chmod("/etc/lnos/known_nodes", 0644);
        }

        return true;
    }

    bool addKnownNode(const KnownNode& node)
    {
        if (geteuid() != 0 || node.name.empty() || !createConfig())
            return false;

        std::ofstream file("/etc/lnos/known_nodes", std::ios::app);
        if (!file.is_open())
            return false;

        file << node.name << ' ' << publicKeyToHex(node.publicKey) << '\n';
        return static_cast<bool>(file);
    }

    std::unordered_map<std::string, KnownNode> loadKnownNodes() {
        std::unordered_map<std::string, KnownNode> knownNodes;

        std::ifstream file("/etc/lnos/known_nodes");

        std::string name;
        std::string key;

        while (file >> name >> key) {
            KnownNode node{};
            node.name = name;

            if (!hexToPublicKey(key, node.publicKey))
                continue;

            knownNodes[name] = node;
        }

        return knownNodes;
    }

}
