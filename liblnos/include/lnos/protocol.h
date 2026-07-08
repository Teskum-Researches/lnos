#pragma once
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/*{
  "version": "1",
  "type": "announce",
  "name": "teskum.pc.gervaty",
  "services": [
    {
      "name": "SSH",
      "port": 22
    },
    {
      "name": "HTTP",
      "port": 80
    },
    {
      "name": "Minecraft",
      "port": 25565
    }
  ]
}*/

namespace lnos {

    struct Service {
        std::string name;
        uint16_t port;
    };

    struct Packet {
        std::string version;
        std::string name;
        std::vector<Service> services;
    };

    inline std::string encode(const Packet& p) {
        json j;

        j["version"] = p.version;
        j["type"] = "announce";
        j["name"] = p.name;

        for (const auto& s : p.services)
        {
            j["services"].push_back({
                {"name", s.name},
                {"port", s.port}
            });
        }

        return j.dump();
    }

    inline Packet decode(const std::string& msg) {
        Packet p;

        auto j = json::parse(msg);

        p.version = j["version"];
        p.name = j["name"];

        for (const auto& s : j["services"])
        {
            p.services.push_back({
                s["name"],
                s["port"]
            });
        }

        return p;
    }

}
