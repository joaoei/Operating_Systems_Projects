#include <cstdio>       //printf
#include <cstring>      //memset
#include <cstdlib>      //exit
#include <netinet/in.h> //htons
#include <arpa/inet.h>  //inet_addr
#include <sys/socket.h> //socket
#include <unistd.h>     //close
#include <thread>
#include <chrono>
#include <unordered_set>
#include <signal.h> // kill()
#include "structs.hpp"

#pragma once

#define PORTNUM 11112
#define IP "127.0.0.1"

class SocketClient {
public:
	SocketClient(std::unordered_set<std::string> & blacklist, int &id);
	~SocketClient();
	
	void sendPacket(const Packet &pack);
	void recvServerResponse(std::unordered_set<std::string> &blacklist);

private:
	struct sockaddr_in endereco;
	int socketId;
	int bytesenviados;
};