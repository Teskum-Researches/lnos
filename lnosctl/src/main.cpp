#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sodium.h>
#include <lnos/config.h>
#include <libintl.h>
#include <locale.h>

#define _(string) gettext(string)

bool writeKey(const char* path, const unsigned char* key, std::size_t size)
{
    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cerr << _("Cannot open ") << path << _(" for writing\n");
        return false;
    }

    file.write(reinterpret_cast<const char*>(key),
               static_cast<std::streamsize>(size));

    if (!file) {
        std::cerr << _("Cannot write ") << path << '\n';
        return false;
    }

    file.close();
    if (!file) {
        std::cerr << _("Cannot close ") << path << '\n';
        return false;
    }

    return true;
}

bool generateKeys() {
    if (sodium_init() < 0) {
        std::cerr << _("Failed to initialize libsodium!") << std::endl;
        return false;
    } if (geteuid() != 0) {
        std::cerr << _("Must be run as root") << std::endl;
        return false;
    }

    unsigned char publicKey[crypto_sign_PUBLICKEYBYTES];
    unsigned char privateKey[crypto_sign_SECRETKEYBYTES];

    crypto_sign_keypair(publicKey, privateKey);

    if (!writeKey("/etc/lnos/public.key", publicKey, crypto_sign_PUBLICKEYBYTES)) {
        std::cerr << _("Failed to write public key") << std::endl;
        return false;
    }
    if (!writeKey("/etc/lnos/private.key", privateKey, crypto_sign_SECRETKEYBYTES)) {
        std::cerr << _("Failed to write private key") << std::endl;
        return false;
    }
    return true;
}

void printUsage(char *program_name) {
    std::cerr << _("Usage: ") << program_name << _(" <command>\n");
    std::cerr << _("Available commands:\n");
    std::cerr << _("    generatekeys          generate private and public keys for LNOS node\n");
    std::cerr << _("    init                  create LNOS config\n");
    std::cerr << _("    config                print LNOS config\n");
    std::cerr << _("    set                   override LNOS config property\n");
    std::cerr << _("    get                   print LNOS config property\n");
}

int main(int argc, char** argv)
{
    setlocale(LC_ALL, "");

    bindtextdomain("lnos", LOCALEDIR);
    textdomain("lnos");

    auto cfg = lnos::loadConfig();

    if (argc < 2)
    {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    if (command == "generatekeys") {
        if (geteuid() == 0) {
            generateKeys();
        } else {
            std::cerr << _("Must be run as root") << std::endl;
            return -1;
        }
    } else if (command == "init") {
        if (geteuid() != 0) {
            std::cerr << _("Must be run as root") << std::endl;
            return 1;
        }

        lnos::createConfig();
        return 0;
    } else if (command == "config") {
        std::cout << _("Node Name: ") << cfg.name << std::endl;

        return 0;
    } else if (command == "set") {
        if (geteuid() != 0) {
            std::cerr << _("Must be run as root") << std::endl;
            return 1;
        }
        if (argc < 4) {
            std::cerr << _("Not enough arguments") << std::endl;
            return 1;
        }

        std::string key = argv[2];
        std::string value = argv[3];

        lnos::setConfig(key, value);

        return 0;
    } else if (command == "get") {
        if (argc < 3) {
            std::cerr << _("Not enough arguments") << std::endl;
            return 1;
        }

        std::string key = argv[2];

        if (key == "name") {
            std::cout << _("Node Name: ") << cfg.name << std::endl;
        }
        return 0;
    } else {
        printUsage(argv[0]);
        std::cerr << _("Unknown subcommand ") << command << std::endl;
    }
}
