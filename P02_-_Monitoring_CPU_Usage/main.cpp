#include <algorithm>
#include <vector>
#include <unordered_map>
#include <queue>
#include <fstream>
#include <iostream>
#include <sstream>
#include <dirent.h>

// sleep
#include <chrono>
#include <thread>

#include <signal.h> // kill()

#include "gpio.hpp"

struct Process {
	Process(const unsigned int &_pid, const float &_cpuPerc) {
		pid = _pid;
		cpuUsagePercent = _cpuPerc;
	}

	unsigned int pid;
	float cpuUsagePercent;
};

struct ProcessCompare {
	inline bool operator()(const Process &a, const Process &b) {
		return a.cpuUsagePercent < b.cpuUsagePercent;
	}
};

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
	std::vector<unsigned int> results;
	std::unordered_map<unsigned int, unsigned int> timetable; // <PID, totaltime>
	std::priority_queue<Process, std::vector<Process>, ProcessCompare> processRanking;
	unsigned int curr_cpujiffies, old_cpujiffies = 0, processDelta, totalProcessDelta, cpuDelta;

	bbb::GPIO led_G(51);
	bbb::GPIO led_Y(50);
	bbb::GPIO led_R(60);
	bbb::GPIO button(115);

	led_G.setModeOut();
	led_Y.setModeOut();
	led_R.setModeOut();
	button.setModeIn();


	while (true) {
		std::ifstream cpustatfile("/proc/stat");
		char cpustatchar[1000];
		cpustatfile.getline(cpustatchar, 1000);
		std::stringstream cpustat(cpustatchar+3);

		unsigned int temp;
		curr_cpujiffies = 0;
		while (cpustat >> temp)
			curr_cpujiffies += temp;

		cpuDelta = (curr_cpujiffies - old_cpujiffies);
		old_cpujiffies = curr_cpujiffies;
		totalProcessDelta = 0;
		
		results = readDirectoryForProcesses("/proc");
		for (auto &proc : results) {
			std::string filename("/proc/" + std::to_string(proc) + "/stat");
			std::ifstream fs(filename);
			if (!fs.is_open()) continue;
			std::string read;

			for (int i = 0; i < 13; ++i)
				fs >> read;

			unsigned int totaltime = 0;
			for (int i = 0; i < 2; ++i) {
				fs >> read;
				totaltime += std::stoi(read);
			}

			auto it = timetable.find(proc);
			if (it == timetable.end()) {
				timetable.insert({proc, totaltime});
			} else {
				processDelta = totaltime - it->second;
				totalProcessDelta += processDelta;
				float cpuUsageResult = (float) 100 * processDelta/cpuDelta;
				
				Process p(proc, cpuUsageResult);
				processRanking.push(p);

				it->second = totaltime;
			}
		}

		float totalCPUuse = 100 * (float)totalProcessDelta/cpuDelta;
		std::cout << "TOTAL CPU USE: " << totalCPUuse << "%\n";

		if (totalCPUuse > 75) {
			int i;
			for(i = 0; i < 5 and !button.getValue(); i++) {
				led_G.setValueOff();
				led_Y.setValueOff();
				led_R.setValueOff();
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
				led_G.setValueOn();
				led_Y.setValueOn();
				led_R.setValueOn();
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
			}
			// emergency stop
			if (i != 5) {
				led_G.setValueOff();
				led_Y.setValueOff();
				led_R.setValueOff();
				kill(processRanking.top().pid, SIGTERM);
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));
			}
		} else {
			if (totalCPUuse > 50) {
				led_G.setValueOn();
				led_Y.setValueOn();
				led_R.setValueOn();
			} else if (totalCPUuse > 25) {
				led_G.setValueOn();
				led_Y.setValueOn();
				led_R.setValueOff();
			} else {
				led_G.setValueOn();
				led_Y.setValueOff();
				led_R.setValueOff();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}

		// debug
		// for (int i = 0; i < 5 and !processRanking.empty(); ++i) {
		// 	Process p = processRanking.top();
		// 	std::cout << "PID: " << p.pid << " / CPU use: " << p.cpuUsagePercent << "\n";
		// 	processRanking.pop();
		// }

		while (!processRanking.empty()) {
			processRanking.pop();
		}
	}

	return 0;
}