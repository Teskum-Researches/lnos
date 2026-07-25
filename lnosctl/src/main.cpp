#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sodium.h>
#include <lnos/config.h>


bool writeKey(const char* path, const unsigned char* key, std::size_t size) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cerr << "Cannot open " << path << " for writing\n";
        return false;
    }

    file.write(reinterpret_cast<const char*>(key),
               static_cast<std::streamsize>(size));

    if (!file) {
        std::cerr << "Cannot write " << path << '\n';
        return false;
    }

    file.close();
    if (!file) {
        std::cerr << "Cannot close " << path << '\n';
        return false;
    }

    return true;
}

bool generateKeys() {
    if (sodium_init() < 0) {
        std::cerr << "Failed to initialize libsodium!" << std::endl;
        return false;
    }
    unsigned char publicKey[crypto_sign_PUBLICKEYBYTES];
    unsigned char privateKey[crypto_sign_SECRETKEYBYTES];

    crypto_sign_keypair(publicKey, privateKey);

    if (!writeKey("/etc/lnos/public.key", publicKey, crypto_sign_PUBLICKEYBYTES)) {
        std::cerr << "Failed to write public key" << std::endl;
        return false;
    }
    if (!writeKey("/etc/lnos/private.key", privateKey, crypto_sign_SECRETKEYBYTES)) {
        std::cerr << "Failed to write private key" << std::endl;
        return false;
    }
    return true;
}

void printUsage(const std::string& programName) {
    std::cout << "Usage: " << programName << " <command>\n\n";

    std::cout << "Commands:\n";
    std::cout << "  * generatekeys   Generate public and private keys.\n";
    std::cout << "  * init           Create the initial LNOS configuration.\n";
    std::cout << "    config         Print the current configuration.\n";
    std::cout << "  * set            Set a configuration property.\n";
    std::cout << "    get            Get a configuration property.\n\n";

    std::cout << "* Root privileges required.\n";
}

int main(int argc, char** argv) {
    auto cfg = lnos::loadConfig();

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    } else if (strcmp(argv[1], "help") == 0) {
        printUsage(argv[0]);
        return 0;
    }

    std::string command = argv[1];
    if (command == "generatekeys") {
        if (geteuid() == 0) {
            generateKeys();
        } else {
            std::cerr << "Must be run as root" << std::endl;
            return -1;
        }
    } else if (command == "init") {
        if (geteuid() != 0) {
            std::cerr << "Must be run as root" << std::endl;
            return 1;
        }

        lnos::createConfig();
        return 0;
    } else if (command == "config") {
        std::cout << "Node Name: " << cfg.name << std::endl;

        return 0;
    } else if (command == "set") {
        if (geteuid() != 0) {
            std::cerr << "Must be run as root" << std::endl;
            return 1;
        }
        if (argc < 4) {
            std::cerr << "Not enough arguments" << std::endl;
            return 1;
        }

        std::string key = argv[2];
        std::string value = argv[3];

        lnos::setConfig(key, value);

        return 0;
    } else if (command == "get") {
        if (argc < 3) {
            std::cerr << "Not enough arguments" << std::endl;
            return 1;
        }

        std::string key = argv[2];

        if (key == "name") {
            std::cout << "Node Name: " << cfg.name << std::endl;
        }
        return 0;
    } else {
        printUsage(argv[0]);
        std::cerr << "Unknown command '" << command << "'" << std::endl;
    }
}
