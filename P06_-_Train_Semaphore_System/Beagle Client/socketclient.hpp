#include <cstdio>       //printf
#include <cstring>      //memset
#include <cstdlib>      //exit
#include <netinet/in.h> //htons
#include <arpa/inet.h>  //inet_addr
#include <sys/socket.h> //socket
#include <unistd.h>     //close
#include <thread>
#include <chrono>

#pragma once

#define PORTNUM 11111

// if target == -1 and toggle, toggle all.
struct Packet {
	Packet(const bool &tog, const int &tar, const int &speed) : 
		   toggle(tog), target(tar), changeSpeed(speed) {}

	bool toggle;
	int target;
	int changeSpeed;
};

class SocketClient {
public:
SocketClient();
~SocketClient();

void sendPacket(const Packet &pack);

private:
	struct sockaddr_in endereco;
	int socketId;
	int bytesenviados;
};