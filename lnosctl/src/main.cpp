#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sodium.h>
#include <lnos/config.h>


bool writeKey(const char* path, const unsigned char* key, std::size_t size)
{
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
        std::cout << "Failed to initialize libsodium!" << std::endl;
        return false;
    } if (geteuid() != 0) {
        std::cout << "Must be run as root" << std::endl;
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

int main(int argc, char** argv)
{
    auto cfg = lnos::loadConfig();

    if (argc < 2)
    {
        std::cout << "Usage: lnosctl <command>\n";
        return 1;
    }

    if (argv[1] == "generatekeys") {
        if (geteuid() == 0) {
            generateKeys();
        } else {
            std::cout << "Must be run as root" << std::endl;
            return -1;
        }
    }
    std::string command = argv[1];

    if (command == "init") {
        if (geteuid() != 0) {
            std::cout << "Must be run as root" << std::endl;
            return 1;
        }

        lnos::createConfig();
        return 0;
    } else if (command == "config") {
        std::cout << "Node Name: " << cfg.name << std::endl;

        return 0;
    } else if (command == "set") {
        if (geteuid() != 0) {
            std::cout << "Must be run as root" << std::endl;
            return 1;
        }
        if (argv[2] == nullptr || argv[3] == nullptr) {
            std::cerr << "Not enough arguments" << std::endl;
            return 1;
        }

        std::string key = argv[2];
        std::string value = argv[3];

        lnos::setConfig(key, value);

        return 0;
    } else if (command == "get") {
        if (argv[2] == nullptr) {
            std::cerr << "Not enough arguments" << std::endl;
            return 1;
        }

        std::string key = argv[2];

        if (key == "name") {
            std::cout << "Node Name: " << cfg.name << std::endl;
        }
        return 0;
    }
}
