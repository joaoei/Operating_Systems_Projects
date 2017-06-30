// Depois ver o que não estiver usando para apagar
#include <cstdlib>
#include <algorithm>

#include <unordered_map>
#include <unordered_set>
#include <queue>

#include <fstream>
#include <iostream>
#include <sstream>
#include <dirent.h>

// sleep
#include <chrono>
#include <thread>

#include <signal.h> // kill()

#include "socketclient.hpp"
#include "structs.hpp"

// Verifies if a word is a number.
bool isdigit(const std::string& str) {
	return (str.front() >= '0' and str.front() <= '9');
}

std::vector<unsigned int> readDirectoryForProcesses(const std::string& path = std::string()) {
	std::vector<unsigned int> result;
	dirent* de;
	DIR* dp;
	errno = 0;
	dp = opendir(path.empty() ? "." : path.c_str());
	
	if (dp) {
		while (true) {
			errno = 0;
			de = readdir(dp);
			if (de == NULL) break;
			if (isdigit(de->d_name[0])){
				result.push_back(std::stoi(de->d_name));
			}
		}
		closedir(dp);
	}

	return result;
}

int main(int argc, char const *argv[]) {
	char lineread[1000];
	
	// Data files
	std::vector<unsigned int> results;
	std::unordered_map<unsigned int, unsigned int> timetable; // <PID, totaltime>
	std::priority_queue<Process, std::vector<Process>, ProcessCompare> processRanking;
	unsigned int curr_cpujiffies, old_cpujiffies = 0, processDelta, totalProcessDelta, cpuDelta;
	
	std::unordered_set<std::string> blacklist;
	int client_id;
	
	// Get client id from file.
	std::ifstream readidfile("client_id.txt");
	if (!readidfile.is_open())
		client_id = 0;
	else {
		std::string sID;
		readidfile >> sID;
		client_id = stoi(sID);
	}
	readidfile.close();
	
	// Create socket for server communication

	SocketClient socketclient(blacklist, client_id);
	// Update file with the client id
	std::ofstream writeidfile("client_id.txt", std::ofstream::trunc);
	writeidfile << client_id;
	writeidfile.close();
	
	// Total memory available
	float memTotal;
	std::ifstream memstatfile("/proc/meminfo");
	std::string sMemTotal;
	
	memstatfile >> sMemTotal;
	memstatfile >> sMemTotal;
	
	memTotal = stof(sMemTotal);
	memstatfile.close();

	while (true) {
		/***** CPU USAGE CALCULATION *****/
		
		// Reads /proc/stat for cpu info
		std::ifstream cpustatfile("/proc/stat");
		char cpustatchar[1000];
		cpustatfile.getline(cpustatchar, 1000);
		std::stringstream cpustat(cpustatchar+3);

		// Calculates cpu use
		unsigned int temp;
		curr_cpujiffies = 0;
		while (cpustat >> temp)
			curr_cpujiffies += temp;

		// Calculates the cpu usage variation
		cpuDelta = (curr_cpujiffies - old_cpujiffies);
		old_cpujiffies = curr_cpujiffies;
		totalProcessDelta = 0;
		
		/***** MEMORY USAGE CALCULATION *****/
		
		std::ifstream memfile("/proc/meminfo");
	
		memfile.getline(lineread, 1000);
		std::string sMemFree;
		
		memfile >> sMemFree;
		memfile >> sMemFree;
		//memory used to be sent via packet
		float totalMemuse = 100 * ( memTotal - stof(sMemFree) ) / memTotal;
		memfile.close();
		
		/***** PROCESS LOOP *****/
		results = readDirectoryForProcesses("/proc");
		for (auto &proc : results) {
			Process p;
			p.pid = proc;
			std::string file_mem("/proc/" + std::to_string(proc) + "/status"); // memory usage
			std::string file_cpu("/proc/" + std::to_string(proc) + "/stat"); // cpu usage
			
			/***** PROCESS MEMORY USAGE CALCULATION *****/
			std::ifstream fs(file_mem);
			if (!fs.is_open()) continue;
			
			std::string procName;
			fs >> procName;
			fs >> procName;
			
			// If on blacklist, kill it!
			auto itBl = blacklist.find(procName);
			if(itBl != blacklist.end()){
				kill(proc, SIGTERM);
				continue;
			}
			
			strcpy(p.name, procName.c_str());
			fs.getline(lineread, 1000); // discard rest of the line
			
			for (int i = 0; i < 15; ++i)
				fs.getline(lineread, 1000); // 15 discards

			// VmSize
			// fs.getline(lineread, 1000);
			std::string sProcMemV;
			fs >> sProcMemV;
	
			if (sProcMemV.compare("VmSize:") == 0){
				fs >> sProcMemV;
				p.memV = stof(sProcMemV) / 1024;
			}
			fs.getline(lineread, 1000); // discard rest of the line
	
			for (int i = 0; i < 3; ++i)
				fs.getline(lineread, 1000); // 3 discards
	
			// VmRSS
			// fs.getline(lineread, 1000);
			std::string sProcMemRSS;
			fs >> sProcMemRSS;
	
			if (sProcMemRSS.compare("VmRSS:") == 0){
				fs >> sProcMemRSS;
				p.memRSS = stof(sProcMemRSS) / 1024;
			}
			fs.getline(lineread, 1000); // discard rest of the line
			
			fs.close();
			
			/***** PROCESS CPU USAGE CALCULATION *****/
			fs.open(file_cpu);
			std::string read;

			// reads the 14th and 15th info stored on them (usertime and kerneltime on cpu)
			for (int i = 0; i < 13; ++i)
				fs >> read;

			unsigned int totaltime = 0;
			for (int i = 0; i < 2; ++i) {
				fs >> read;
				totaltime += std::stoi(read);
			}

			// stores process if first time read, otherwise calculates cpu usage variation
			// and percentage.
			auto itTm = timetable.find(proc);
			if (itTm == timetable.end()) {
				timetable.insert({proc, totaltime});
			} else {
				processDelta = totaltime - itTm->second;
				totalProcessDelta += processDelta;
				float cpuUsageResult = (float) 100 * processDelta/cpuDelta;
				
				p.cpuUsagePercent = cpuUsageResult;
				itTm->second = totaltime;
			}
			
			fs.close();
			
			processRanking.push(p);
		}
		

		// calculates total cpu usage
		float totalCPUuse = 100 * (float)totalProcessDelta/cpuDelta;
		
		Process top_processes[25];
		
		// system("clear;clear;");

		for (int i = 0; i < 25 and !processRanking.empty(); i++) {
			top_processes[i] = processRanking.top();
			processRanking.pop();
			// std::cout << i << " -- pid " << top_processes[i].pid;
			// std::cout << " nome " << top_processes[i].name;
			// std::cout << " cpu " << top_processes[i].cpuUsagePercent;
			// std::cout << " memV " << top_processes[i].memV;
			// std::cout << " memRSS " << top_processes[i].memRSS << "\n";
		}
		
		// std::cout << "CPU use " << totalCPUuse << "%\n";
		// std::cout << "MEM use " << totalMemuse << "%\n";
		
		Packet pack_send(totalCPUuse, totalMemuse, top_processes);
		socketclient.sendPacket(pack_send);
		socketclient.recvServerResponse(blacklist);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}