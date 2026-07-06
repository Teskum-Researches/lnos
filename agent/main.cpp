#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../shared/protocol.h"

#define SERVER_IP "127.0.0.1"
#define PORT 4545

std::string myName = "pc.main";
std::string myIp = "192.168.0.10";

void sender() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    while (true) {
        lnos::Packet p{myName, myIp};
        std::string msg = lnos::encode(p);

        sendto(sock,
               msg.c_str(),
               msg.size(),
               0,
               (sockaddr*)&server,
               sizeof(server));

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}

int main() {
    sender();
}