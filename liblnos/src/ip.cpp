#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <lnos/ip.h>

std::string getip(std::string interface)
{
    struct ifreq ifr{};
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, interface.c_str());

    int s = socket(AF_INET, SOCK_DGRAM, 0);
    ioctl(s, SIOCGIFADDR, &ifr);
    close(s);

    struct sockaddr_in *sa = (struct sockaddr_in*)&ifr.ifr_addr;
    std::string ipAdress = inet_ntoa(sa->sin_addr);
    // printf("addr = %s\n", inet_ntoa(sa->sin_addr));
    return ipAdress;
}

