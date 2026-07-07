#include <fstream>
#include <lnos/config.h>
#include <sys/stat.h>
#include <unistd.h>


namespace lnos {

    Config loadConfig()
    {
        Config cfg;

        std::ifstream nameFile("/etc/lnos/name");

        if (nameFile.is_open()) {
            nameFile >> cfg.name;
        }


        std::ifstream interfaceFile("/etc/lnos/interface");

        if (interfaceFile.is_open()) {
            interfaceFile >> cfg.interface;
        }

        return cfg;
    }


    bool setConfig(const std::string& key, const std::string& value)
    {
        if (geteuid() != 0) {
            return false;
        }

        if (key == "name") {

            std::ofstream file("/etc/lnos/name");

            if (file.is_open()) {
                file << value;
                return true;
            }

            return false;
        }

        if (key == "interface") {

            std::ofstream file("/etc/lnos/interface");

            if (file.is_open()) {
                file << value;
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
            }

            return "";
        }

        if (key == "interface") {

            std::ifstream file("/etc/lnos/interface");

            if (file.is_open()) {
                std::string value;
                file >> value;
            }

            return "";
        }

        return "";
    }


    bool createConfig()
    {
        mkdir("/etc/lnos", 0755);

        std::ofstream nameFile("/etc/lnos/name");

        if (!nameFile.is_open())
            return false;

        nameFile << "default.node";

        std::ofstream interfaceFile("/etc/lnos/interface");

        if (!interfaceFile.is_open())
            return false;

        interfaceFile << "eth0";

        chmod("/etc/lnos", 0755);
        chmod("/etc/lnos/name", 0644);
        chmod("/etc/lnos/interface", 0644);

        return true;
    }

}