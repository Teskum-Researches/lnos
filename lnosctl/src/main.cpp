#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sodium.h>
#include <lnos/config.h>
#include <libintl.h>
#include <clocale>
#include <sys/un.h>
#include <sys/socket.h>

#define _(string) gettext(string)

bool writeKey(const char* path, const unsigned char* key, std::size_t size) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file.is_open()) {
        std::cerr << _("Cannot open ") << path << _(" for writing") << std::endl;
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

void printUsage(const std::string& programName) {
    std::cout << _("Usage: ") << programName << _(" <command>") << "\n\n";

    std::cout << _("Commands:") << "\n" ;
    std::cout << _("  * generatekeys   Generate public and private keys.") << std::endl;
    std::cout << _("  * init           Create the initial LNOS configuration.") << std::endl;
    std::cout << _("    config         Print the current configuration.") << std::endl;
    std::cout << _("  * set            Set a configuration property.") << std::endl;
    std::cout << _("    get            Get a configuration property.") << "\n\n";
    std::cout << _("    send           Send message to node") << std::endl;
    std::cout << _("  * nodes          List all nodes.") << std::endl;

    std::cout << _("* Root privileges required.") << std::endl;
}

int main(int argc, char** argv) {
    setlocale(LC_ALL, "");

    bindtextdomain("lnos", LOCALEDIR);
    bind_textdomain_codeset("lnos", "UTF-8");
    textdomain("lnos");

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
    } else if (command == "nodes") {
        sockaddr_un addr{AF_UNIX};
        strncpy(addr.sun_path, "/run/lnos/lnosd.sock", sizeof(addr.sun_path) - 1);

        int sock = socket(AF_UNIX, SOCK_STREAM, 0);

        if (sock == -1) {
            perror("socket");
            return 1;
        }

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("connect");
            close(sock);
            return 1;
        }

        const char* cmd = "LIST\n";

        send(sock, cmd, strlen(cmd), 0);

        char buffer[1024];

        ssize_t n;
        while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            std::cout.write(buffer, n);
        }
        close(sock);
    } else if (command == "send") {
        if (argc < 2) {
            std::cerr << _("Not enough arguments") << std::endl;
        }
        std::string name = argv[2];
        std::string msg = argv[3];

        sockaddr_un addr{AF_UNIX};
        strncpy(addr.sun_path, "/run/lnos/lnosd.sock", sizeof(addr.sun_path) - 1);

        int sock = socket(AF_UNIX, SOCK_STREAM, 0);

        if (sock == -1) {
            perror("socket");
            return 1;
        }

        if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
            perror("connect");
            close(sock);
            return 1;
        }

        std::string cmd = "SEND " + name + " " + msg + "\n";
        send(sock, cmd.c_str(), strlen(cmd.c_str()), 0);

        char buffer[1024];

        ssize_t n;
        while ((n = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            std::cout.write(buffer, n);
        }
        close(sock);
    }
    else {
        printUsage(argv[0]);
        std::cerr << _("Unknown command '") << command << "'" << std::endl;
    }
}
