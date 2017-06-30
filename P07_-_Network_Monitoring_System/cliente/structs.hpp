#pragma once

#include <ctime>
#include <cstring>

/*
Info da conexão:
pacote mais recente
ip do usuario
id do usuario
tempo de inicio da conexao
*/

/*
Informações do pacote:
coisas dos processos
*/
struct Process {
	unsigned int pid;
	float cpuUsagePercent = 0;
	float memV = 0;
	float memRSS = 0;
	char name[16];
	
	Process& operator=(const Process& pro)
	{
	    pid = pro.pid;
	    cpuUsagePercent = pro.cpuUsagePercent;
	    memV = pro.memV;
	    memRSS = pro.memRSS;
	    // std::copy(pro.name, pro.name + 100, name);
	    strcpy(name, pro.name);
	}
};

// Compare processes by CPU percent usage
struct ProcessCompare {
	bool operator() (const Process& lhs, const Process& rhs) const {
		if(lhs.cpuUsagePercent != rhs.cpuUsagePercent)
			return lhs.cpuUsagePercent < rhs.cpuUsagePercent;
		else
			return lhs.memV < rhs.memV;
	}
};

struct Packet {
	Packet(const float &cpu, const float &mem, const Process *processes) : 
		   totalCPUuse(cpu),
		   totalMEMuse(mem) {
		for (int i = 0; i < 25; i++)
			topProcesses[i]	= processes[i];
	}

	Packet() {}
	
	// it works on real machines
	float totalCPUuse;
	float totalMEMuse;
	Process topProcesses[25];
};

struct ClientInfo {
	unsigned int id;
	unsigned int numberTimesConnected;
	time_t totalConnTime;

	ClientInfo() {
		id = 0;
		numberTimesConnected = 0;
		totalConnTime = 0;
	}
};

struct ConnectionInfo {
	Packet packet;
	unsigned int lastBlacklistOperation;
	int id; // use history to get info
	time_t connectedSince;
	bool isAlive;
};

struct OrderPacket {
	// 1 for kill, 2 for insert blacklist, 3 for remove blacklist
	int operation;
	bool lastPacket;
	unsigned int pid;
	char processName[16] = {'\0'};
};

struct BlacklistOperation {
	bool inserting;
	std::string processName;
	unsigned int opId;
};
