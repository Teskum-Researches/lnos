#include <iostream>
#include <lnos/config.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    auto cfg = lnos::loadConfig();

    if (argc < 2)
    {
        std::cout << "Usage: lnosctl <command>\n";
        return 1;
    }

    std::string command = argv[1];

    if (command == "init") {
        if (geteuid() != 0) return 1;

        lnos::createConfig();
        return 0;
    } else if (command == "config") {
        std::cout << "Node Name: " << cfg.name << std::endl;
        std::cout << "Network Interface: " << cfg.interface << std::endl;

        return 0;
    } else if (command == "set") {
        if (geteuid() != 0) return 1;
        if (argv[2] == nullptr || argv[3] == nullptr) return 1;

        std::string key = argv[2];
        std::string value = argv[3];

        lnos::setConfig(key, value);

        return 0;
    } else if (command == "get") {
        if (argv[2] == nullptr) return 1;

        std::string key = argv[2];

        if (key == "name") {
            std::cout << "Node Name: " << cfg.name << std::endl;
        } else if (key == "interface") {
            std::cout << "Network Interface: " << cfg.interface << std::endl;
        }
        return 0;
    }
}