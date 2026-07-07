#include <fstream>
#include <lnos/config.h>
#include <sys/stat.h>
#include <unistd.h>


namespace lnos {
    Config loadConfig() {
        Config cfg;

        std::ifstream nameFile("/etc/lnos/name");

        if (!nameFile.is_open()) return cfg;

        nameFile >> cfg.nodeName;
        nameFile.close();

        std::ifstream interfaceFile("/etc/lnos/interface");

        if (!interfaceFile.is_open()) return cfg;
        interfaceFile >> cfg.interface;
        interfaceFile.close();

        return cfg;
    }

    void createConfig() {
        if (geteuid() == 0) {
            mkdir("/etc/lnos", 0755);
            std::ofstream file("/etc/lnos/name");
            file << "default.node";
            file.close();

            std::ofstream ifile("/etc/lnos/interface");
            ifile << "eth0";
            ifile.close();

            chmod("/etc/lnos", 0755);
            chmod("/etc/lnos/name", 0644);
            chmod("/etc/lnos/interface", 0644);
        }
    }
}