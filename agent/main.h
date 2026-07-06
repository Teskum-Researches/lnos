#pragma once


#define SERVER_IP "127.0.0.1"

#define MCAST_GROUP "239.255.42.99"
#define PORT 4545

std::string getName();
void sender();
int main();