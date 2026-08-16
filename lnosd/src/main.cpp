#include <iostream>
#include <thread>
#include <csignal>
#include <cerrno>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <mutex>
#include <atomic>
#include <array>
#include <sodium.h>
#include <libintl.h>
#include <clocale>
#include <fcntl.h>
#include <sys/stat.h>
#include <optional>

#include <lnos/crypto.h>
#include "registry.h"
#include <lnos/protocol.h>
#include <lnos/config.h>

#define _(string) gettext(string)

#define MCAST_GROUP "239.255.42.99"
#define MCAST_PORT 4545
#define MESSAGE_PORT 4546

std::atomic<bool> running = true;
int ctlSocket = -1;

void handleSigint(int) {
    std::cout << "CTRL+C received\n";
    running = false;
}

std::array<std::uint8_t, PUBLIC_KEY_SIZE> publicKey;
std::mutex nodesMutex;
std::mutex coutMutex;

lnos::Config cfg;

void stopWithError(const std::string& message) {
    if (!running.exchange(false))
        return;

    std::cerr << _("[fatal] ") << message << "\n"
              << _("LNOS will now shut down.") << "\n";
    exit(EXIT_FAILURE);
}

void stopAfterSystemError(const char* operation) {
    perror(operation);
    stopWithError(std::string(_("Operation failed: ")) + operation);
}

std::optional<std::string> getNodeIP(const std::string& nodeName) {
    std::lock_guard<std::mutex> lock(nodesMutex);

    auto it = nodes.find(nodeName);

    if (it == nodes.end())
        return std::nullopt;

    if (it->second.status != NodeStatus::Online)
        return std::nullopt;

    return it->second.ip;
}

int sendToNode(const std::string& target, const std::string& msg) {
    auto ip = getNodeIP(target);

    if (!ip) {
        std::cerr << _("Node not found or offline: ")
                  << target << '\n';
        return -1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        stopAfterSystemError("socket");
        return -1;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MESSAGE_PORT);

    if (inet_pton(AF_INET, ip->c_str(), &addr.sin_addr) != 1) {
        std::cerr << _("Invalid node IP: ") << *ip << '\n';
        close(sock);
        return -1;
    }

    if (sendto(sock,
               msg.data(),
               msg.size(),
               0,
               reinterpret_cast<sockaddr*>(&addr),
               sizeof(addr)) < 0) {
        perror("sendto");
        close(sock);
        return -1;
               }

    close(sock);
    return 0;
}

bool signPacket(lnos::Packet& packet,
                const std::array<uint8_t, PRIVATE_KEY_SIZE>& privateKey)
{
    lnos::Blob data = lnos::encode(packet, true);

    // Важно: кодируем без signature

    unsigned long long signatureLength;

    crypto_sign_detached(
        packet.signature.data(),
        &signatureLength,
        data.data(),
        data.size(),
        privateKey.data()
    );

    return signatureLength == crypto_sign_BYTES;
}

void sender() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        stopAfterSystemError("sender socket");
        return;
    }

    unsigned char ttl = 1;

    if (setsockopt(sock,
                   IPPROTO_IP,
                   IP_MULTICAST_TTL,
                   &ttl,
                   sizeof(ttl)) < 0) {
        stopAfterSystemError("IP_MULTICAST_TTL");
        close(sock);
        return;
    }

    // Разрешаем получать свои же multicast-пакеты
    int loop = 1;
    if (setsockopt(sock,
                   IPPROTO_IP,
                   IP_MULTICAST_LOOP,
                   &loop,
                   sizeof(loop)) < 0) {
        stopAfterSystemError("IP_MULTICAST_LOOP");
        close(sock);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MCAST_PORT);

    if (inet_pton(AF_INET, MCAST_GROUP, &addr.sin_addr) != 1) {
        stopWithError(_("Invalid multicast IPv4 address: '") + std::string(MCAST_GROUP) + "'");
        close(sock);
        return;
    }

    auto privateKey = lnos::loadPrivateKey();
    auto publicKey = lnos::loadPublicKey();

    while (running) {

        lnos::Packet p(cfg.name, cfg.services);

        p.publicKey = publicKey;

        if (!lnos::signPacket(p, privateKey))
        {
            stopWithError(_("Packet signing failed"));
            break;
        }

        lnos::Blob msg = lnos::encode(p, true);

        std::cout << "[debug] sending "
                  << msg.size()
                  << " bytes\n";

        if (sendto(sock,
                   msg.data(),
                   msg.size(),
                   0,
                   reinterpret_cast<sockaddr*>(&addr),
                   sizeof(addr)) < 0) {
            stopAfterSystemError("sendto");
            break;
        }


        for (int i = 0; i < 20 && running; i++)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(sock);
}

void receiver() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        stopAfterSystemError("receiver socket");
        return;
    }

    int reuse = 1;

    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &reuse,
                   sizeof(reuse)) < 0) {
        stopAfterSystemError("SO_REUSEADDR");
        close(sock);
        return;
    }

    timeval tv{};
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   &tv,
                   sizeof(tv)) < 0) {
        stopAfterSystemError("SO_RCVTIMEO");
        close(sock);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MCAST_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock,
             reinterpret_cast<sockaddr*>(&addr),
             sizeof(addr)) < 0) {

        stopAfterSystemError("bind");
        close(sock);
        return;
    }

    ip_mreq mreq{};

    // multicast адрес
    if (inet_pton(AF_INET,
                  MCAST_GROUP,
                  &mreq.imr_multiaddr) != 1) {
        stopWithError(_("Invalid multicast IPv4 address: '") + std::string(MCAST_GROUP) + "'");
        close(sock);
        return;
    }

    // ОС выберет интерфейс multicast по таблице маршрутизации.
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if (setsockopt(sock,
                   IPPROTO_IP,
                   IP_ADD_MEMBERSHIP,
                   &mreq,
                   sizeof(mreq)) < 0) {

        stopAfterSystemError("IP_ADD_MEMBERSHIP");
        close(sock);
        return;
    }

    char buffer[1024];

    while (running) {

        sockaddr_in senderAddr{};
        socklen_t senderLen = sizeof(senderAddr);


        ssize_t len = recvfrom(sock,
                               buffer,
                               sizeof(buffer) - 1,
                               0,
                               reinterpret_cast<sockaddr*>(&senderAddr),
                               &senderLen);

        if (len < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR) {
                stopAfterSystemError("recvfrom");
                break;
            }
            continue;
        }

        if (len == 0)
            continue;

        buffer[len] = 0;

        char ip[INET_ADDRSTRLEN];

        if (inet_ntop(AF_INET,
                      &senderAddr.sin_addr,
                      ip,
                      sizeof(ip)) == nullptr) {
            stopAfterSystemError("inet_ntop");
            break;
        }


        std::cout << _("[debug] received ")
                  << len
                  << _(" bytes from ")
                  << ip
                  << "\n";

        lnos::EncodedPacket encoded((uint8_t *)buffer, len);
        lnos::Packet p;

        if (!lnos::decode(encoded, p)) {
            std::cerr << "[error] received invalid packet\n";
            return;
        }

        if (!lnos::verifyPacket(p)) {
            std::cerr << "[error] invalid signature\n";
            return;
        }

        if (p.type == lnos::PacketType::Announce) {
            const lnos::KnownNode knownNode{
                p.as.announce.name,
                p.publicKey
            };
            bool saveKnownNode = false;

            {
                std::lock_guard<std::mutex> lock(nodesMutex);

                nodes[p.as.announce.name] = {
                    p.as.announce.name,
                    ip,
                    p.as.announce.services,
                    std::chrono::steady_clock::now(),
                    NodeStatus::Online
                };

                const auto it = knownNodes.find(knownNode.name);
                saveKnownNode = it == knownNodes.end()
                             || it->second.publicKey != knownNode.publicKey;
            }

            // Не записываем один и тот же ключ при каждом Announce.
            if (saveKnownNode) {
                if (!lnos::addKnownNode(knownNode)) {
                    std::cerr << "[error] failed to save known node "
                              << knownNode.name << "\n";
                } else {
                    std::lock_guard<std::mutex> lock(nodesMutex);
                    knownNodes[knownNode.name] = knownNode;
                }
            }
        } else {
            std::cerr << _("[error] received invalid packet") << std::endl;
        }
    }

    close(sock);
}

void messageReceiver() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        stopAfterSystemError("message socket");
        return;
    }

    int reuse = 1;

    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &reuse,
                   sizeof(reuse)) < 0) {
        stopAfterSystemError("SO_REUSEADDR");
        close(sock);
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MESSAGE_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock,
             reinterpret_cast<sockaddr*>(&addr),
             sizeof(addr)) < 0) {
        stopAfterSystemError("message bind");
        close(sock);
        return;
    }

    timeval tv{};
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    if (setsockopt(sock,
                   SOL_SOCKET,
                   SO_RCVTIMEO,
                   &tv,
                   sizeof(tv)) < 0) {
        stopAfterSystemError("SO_RCVTIMEO");
        close(sock);
        return;
    }

    char buffer[4096];

    while (running) {
        sockaddr_in senderAddr{};
        socklen_t senderLen = sizeof(senderAddr);

        ssize_t len = recvfrom(
            sock,
            buffer,
            sizeof(buffer),
            0,
            reinterpret_cast<sockaddr*>(&senderAddr),
            &senderLen
        );

        if (len < 0) {
            if (errno == EAGAIN ||
                errno == EWOULDBLOCK ||
                errno == EINTR) {
                continue;
            }

            stopAfterSystemError("message recvfrom");
            break;
        }

        if (len == 0)
            continue;

        char ip[INET_ADDRSTRLEN];

        if (inet_ntop(AF_INET,
                      &senderAddr.sin_addr,
                      ip,
                      sizeof(ip)) == nullptr) {
            stopAfterSystemError("inet_ntop");
            break;
        }

        std::string message(buffer, len);

        {
            std::lock_guard<std::mutex> lock(coutMutex);

            std::cout << "[message] "
                      << ip
                      << ": "
                      << message
                      << '\n';
        }
    }

    close(sock);
}

void printer() {
    while (running) {

        {
            std::lock_guard<std::mutex> lock(nodesMutex);
            std::cout << _("=== LNOS NODES ===") << std::endl;

                for (const auto& n : nodes) {
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>
                    (std::chrono::steady_clock::now() - n.second.lastSeen).count();

                    std::cout << n.second.name
                              << " - " << n.second.ip
                              << _(" Status: ")
                              << (n.second.status == NodeStatus::Online
                                  ? _("Online")
                                  : _("Offline"));
                    if (n.second.status == NodeStatus::Offline) {
                        std::cout << "(" << seconds << _(" seconds ago)");
                    }
                    std::cout << std::endl;
                    std::cout << _("Services:") << std::endl;

                    for (const auto& s : n.second.services)
                    {
                        std::cout
                            << "  "
                            << s.name
                            << ":"
                            << s.port
                            << '\n';
                    }
                }
        } // mutex освобождён здесь


        for (int i = 0; i < 100 && running; i++)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void cleanup() {
    while (running) {

        {
            std::lock_guard<std::mutex> lock(nodesMutex);

            for (auto& n : nodes) {
                if (std::chrono::steady_clock::now() - n.second.lastSeen
                    > std::chrono::seconds(15)) {
                    n.second.status = NodeStatus::Offline;
                    }
            }
        }

        for (int i = 0; i < 100 && running; i++)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void ctlserver() {
    int server = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server == -1) stopAfterSystemError("socket");
    int flags = fcntl(server, F_GETFL, 0);
    fcntl(server, F_SETFL, flags | O_NONBLOCK);
    ctlSocket = server;
    sockaddr_un server_addr{AF_UNIX};

    strcpy(server_addr.sun_path, "/run/lnos/lnosd.sock");

    unlink("/run/lnos/lnosd.sock");
    if (bind(server, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        stopAfterSystemError("bind");
    }
    chmod("/run/lnos/lnosd.sock", 0666);

    if  (listen(server, SOMAXCONN) == -1) {
        stopAfterSystemError("listen");
    }

    while (running) {
        sockaddr_un client_addr{};
        socklen_t len = sizeof(client_addr);

        int client = accept(server, (sockaddr*)&client_addr, &len);
        if (client == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(100)
                );
                continue;
            }
            if (!running)
                break;

            stopAfterSystemError("accept");
        }
        int cflags = fcntl(client, F_GETFL, 0);
        fcntl(client, F_SETFL, cflags | O_NONBLOCK);

        char buffer[1024];
        ssize_t received = recv(client, buffer, sizeof(buffer), 0);

        if (received <= 0) {
            close(client);
            continue;
        }
        std::string cmd(buffer, received);
        if (cmd == "LIST\n") {
            std::string response;
            std::stringstream rstream;
            {
                std::lock_guard<std::mutex> lock(nodesMutex);
                for (const auto& n : nodes) {
                    auto seconds = std::chrono::duration_cast<std::chrono::seconds>
                    (std::chrono::steady_clock::now() - n.second.lastSeen).count();
                    rstream << n.second.name
                              << " - " << n.second.ip
                              << _(" Status: ")
                              << (n.second.status == NodeStatus::Online
                                  ? _("Online")
                                  : _("Offline"));
                    if (n.second.status == NodeStatus::Offline) {
                        rstream << "(" << seconds << _(" seconds ago") << "";
                    }
                    rstream << std::endl;
                    rstream << _("Services:") << std::endl;

                    for (const auto& s : n.second.services) {
                        rstream << "  " << s.name << ":" << s.port << '\n';
                    }
                }
                rstream << std::endl;
                response += rstream.str();
            }
            send(client, response.data(), response.size(), 0);
            close(client);
        } else if (cmd.starts_with("SEND")) {
            size_t first = cmd.find(' ');
            size_t second = cmd.find(' ', first + 1);

            if (second == std::string::npos) {
                return;
            }

            std::string target = cmd.substr(first + 1, second - first - 1);
            std::string msg = cmd.substr(second + 1);

            if (sendToNode(target, msg) == 0) {
                std::string response = "Sent " + msg + " to " + target + "\n";
                send(client, response.data(), response.size(), 0);
                close(client);
            }
        }
    }
    close(server);
    unlink("/run/lnos/lnosd.sock");
}

int main() {
    setlocale(LC_ALL, "");

    bindtextdomain("lnos", LOCALEDIR);
    bind_textdomain_codeset("lnos", "UTF-8");
    textdomain("lnos");

    std::signal(SIGINT, handleSigint);
    std::signal(SIGTERM, handleSigint);

    if (geteuid() != 0) {
        std::cerr << "warning: lnosd is not running as root" << std::endl;
    } else {
        lnos::createConfig();
    }

    cfg = lnos::loadConfig();
    knownNodes = lnos::loadKnownNodes();

    std::thread t1(sender);
    std::thread t2(receiver);
    std::thread t3(printer);
    std::thread t4(cleanup);
    std::thread t5(ctlserver);
    std::thread t6(messageReceiver);

    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    std::cout << _("LNOS is stopped.") << std::endl;
}
