#include <cstring>

#pragma once 

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