#include <cstdlib>
#include <vector>
#include <list>
#include <set>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <dirent.h>
 
// For sleep
#include <thread>
#include <chrono>

// username
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

// float setprecision()
#include <iomanip>

// struct to store process info
struct Process {
	std::string name;
	std::string user;
	int pid;
	int ppid;
	float memV = 0;
	float memRSS = 0;
	float swap = 0;
	long unsigned int major_fault;
	long unsigned int minor_fault;
};

struct ProcessComp {
	int compareType;

	ProcessComp(const int &_compareType) {
		this->compareType = _compareType;
	}

	bool operator() (const Process& lhs, const Process& rhs) const {
		switch (compareType) {
		  case 1:
			return lhs.pid > rhs.pid;
		  break;
		  case 2:
		  	return lhs.memV > rhs.memV;
		  break;
		  case 3:
		  	return lhs.swap > rhs.swap;
		  break;
		  case 4:
		  	return lhs.major_fault > rhs.major_fault;
		  break;
		  case 5:
		  	return lhs.minor_fault > rhs.minor_fault;
		  break;
		  default:
		  	return lhs.memRSS > rhs.memRSS;
		  break;
		}
	}
};

// function signatures
// ----------------------------------------------------------------------------
bool isdigit(const std::string& str);
std::vector<std::string> readDirectoryForProcesses(const std::string& path);
void parseResults(const std::vector<std::string>& processes, const int& targetComp, const int& numProc);
// ----------------------------------------------------------------------------

bool isdigit(const std::string& str) {
	return (str.front() >= '0' and str.front() <= '9');
}

// Read the processes from the /proc folder
std::vector<std::string> readDirectoryForProcesses(const std::string& path = std::string()) {
	std::vector <std::string> result;
	dirent* de;
	DIR* dp;
	errno = 0;
	dp = opendir(path.empty() ? "." : path.c_str());
	
	if (dp) {
		while (true) {
			errno = 0;
			de = readdir(dp);
			if (de == NULL) break;
			if (isdigit(de->d_name[0]))
				result.push_back(std::string(de->d_name));
		}
		closedir(dp);
	}

	return result;
}

void parseResults(const std::vector<std::string>& processes, const int& targetComp, const int& numProc) {
	char temp[1000];
	int procToShow = processes.size();

	ProcessComp my_comp(targetComp);
	std::multiset<Process, ProcessComp> process_set(my_comp);

	std::string filename("/proc/meminfo");
	std::ifstream fs(filename);

	std::cout << std::setprecision(2) << std::fixed;

	//Total memory
	fs.getline(temp, 1000);
	std::string sMemTotal(temp);
	sMemTotal.erase(sMemTotal.size() - 3, 3);
	std::size_t first_blank = sMemTotal.find_last_of(" ") + 1;
	sMemTotal.erase(0, first_blank);
	float memTotal = stof(sMemTotal)/1024;
	std::cout << "Main memory available -> " << memTotal << " MB\n";

	// Memory used
	fs.getline(temp, 1000);
	std::string sMemFree(temp);
	sMemFree.erase(sMemFree.size() - 3, 3);
	first_blank = sMemFree.find_last_of(" ") + 1;
	sMemFree.erase(0, first_blank);
	float memUsed = stof(sMemTotal) - stof(sMemFree);
	std::cout << "Main memory in use -> " << memUsed/1024 << " MB (" 
				<< ((memUsed*100)/memTotal)/1024 << "%)\n\n";

	// Cached memory
	fs.getline(temp, 1000); // discarded
	fs.getline(temp, 1000); // discarded
	fs.getline(temp, 1000);
	std::string sCacheTotal(temp);
	sCacheTotal.erase(sCacheTotal.size() - 3, 3);
	first_blank = sCacheTotal.find_last_of(" ") + 1;
	sCacheTotal.erase(0, first_blank);
	float cacheTotal = stof(sCacheTotal)/1024;
	std::cout << "Cache in use -> " << cacheTotal << " MB\n\n";

	for (int i = 0; i < 9; ++i)
		fs.getline(temp, 1000); // 9 discards

	// Swap total
	fs.getline(temp, 1000);
	std::string sSwapTotal(temp);
	sSwapTotal.erase(sSwapTotal.size() - 3, 3);
	first_blank = sSwapTotal.find_last_of(" ") + 1;
	sSwapTotal.erase(0, first_blank);
	float swapTotal = stof(sSwapTotal)/1024;
	std::cout << "Swap available -> " << swapTotal << " MB\n";

	// Swap used
	fs.getline(temp, 1000);
	std::string sSwapFree(temp);
	sSwapFree.erase(sSwapFree.size() - 3, 3);
	first_blank = sSwapFree.find_last_of(" ") + 1;
	sSwapFree.erase(0, first_blank);
	float swapUsed = stof(sSwapTotal) - stof(sSwapFree);
	std::cout << "Swap in use -> " << swapUsed/1024 << " MB (" 
				<< ((swapUsed*100)/swapTotal)/1024 << "%)\n\n";

	fs.close();

	if (numProc > 0 and numProc <= processes.size())
		procToShow = numProc;

	std::cout << "Showing " << procToShow <<" PROCESSES sorted by ";

	switch (targetComp) {
	  case 1:
		std::cout << "PID\n\n";
	  break;
	  case 2:
	  	std::cout << "VIRTUAL MEMORY\n\n";
	  break;
	  case 3:
	  	std::cout << "SWAP MEMORY\n\n";
	  break;
	  case 4:
	  	std::cout << "MAJOR page FAULTS\n\n";
	  break;
	  case 5:
	  	std::cout << "MINOR page FAULTS\n\n";
	  break;
	  default:
	  	std::cout << "RSS MEMORY (physical memory)\n\n";
	  break;
	}

	std::cout.width(5);
	std::cout << std::right << "PID";

	std::cout.width(20);
	std::cout << std::left << " NAME";

	std::cout.width(20);
	std::cout << std::left << "USER";

	std::cout.width(18);
	std::cout << std::left << "VIRTUAL_MEM(mB)";

	std::cout.width(14);
	std::cout << std::left << "RSS_MEM(mB)";

	std::cout.width(14);
	std::cout << std::left << "SWAP(mB)";

	std::cout.width(14);
	std::cout << std::left << "MAJOR_FAULTS";

	std::cout.width(14);
	std::cout << std::left << "MINOR_FAULTS" << std::endl;

	std::cout << "-----------------------------------------------------------";
	std::cout << "------------------------------------------------------------" << std::endl;

	for(const auto& proc : processes) {
		std::string filename("/proc/" + proc + "/status"); // mem, swap
		std::string filename2("/proc/" + proc + "/stat"); // page faults
		std::ifstream fs(filename);
		if (!fs.is_open()) continue;

		Process p;

		// Name
		fs.getline(temp, 1000);
		std::string sName(temp);
		p.name = sName.substr(6);

		// User
		struct stat info;
		stat(filename.c_str(), &info);
		struct passwd *pw = getpwuid(info.st_uid);
		p.user = std::string(pw->pw_name);

		// PID
		fs.getline(temp, 1000); // discarded
		fs.getline(temp, 1000); // discarded
		fs.getline(temp, 1000); // discarded
		fs.getline(temp, 1000);
		std::string sPID(temp);
		p.pid = std::stoi(sPID.substr(5));

		// PPID 	
		fs.getline(temp, 1000);
		std::string sPPID(temp);
		p.ppid = std::stoi(sPPID.substr(6));

		for (int i = 0; i < 10; ++i)
			fs.getline(temp, 1000); // 10 discards

		// VmSize
		fs.getline(temp, 1000);
		std::string sProcMemV(temp);

		if (sProcMemV.find("VmSize") != std::string::npos){
			sProcMemV.erase(sProcMemV.size() - 3, 3);
			first_blank = sProcMemV.find_last_of(" ") + 1;
			sProcMemV.erase(0, first_blank);
			p.memV = stof(sProcMemV);
		}

		for (int i = 0; i < 3; ++i)
			fs.getline(temp, 1000); // 3 discards

		// VmRSS
		fs.getline(temp, 1000);
		std::string sProcMemRSS(temp);

		if (sProcMemRSS.find("VmRSS") != std::string::npos){
			sProcMemRSS.erase(sProcMemRSS.size() - 3, 3);
			first_blank = sProcMemRSS.find_last_of(" ") + 1;
			sProcMemRSS.erase(0, first_blank);
			p.memRSS = stof(sProcMemRSS);
		}

		for (int i = 0; i < 6; ++i)
			fs.getline(temp, 1000); // 6 discards

		//VmSwap
		fs.getline(temp, 1000);
		std::string sProcSwap(temp);

		if (sProcSwap.find("VmSwap") != std::string::npos){
			sProcSwap.erase(sProcSwap.size() - 3, 3);
			first_blank = sProcSwap.find_last_of(" ") + 1;
			sProcSwap.erase(0, first_blank);
			p.swap = stof(sProcSwap);
		}

		// Page faults
		std::ifstream fs_2(filename2);
		if (!fs_2.is_open()) continue;

		std::string sPage;

		for (int i = 0; i < 9; ++i)
			fs_2 >> sPage; // 9 discards

		fs_2 >> sPage;	
		p.minor_fault = stoul(sPage);
		fs_2 >> sPage;	
		fs_2 >> sPage;	
		p.major_fault = stoul(sPage);

		// Inserting on maps
		process_set.insert(p);

		fs_2.close();
		fs.close();
	}

	int count = 0;
	for (auto &p : process_set) {
		if(count >= procToShow)
			break;
		std::cout.width(5);
		std::cout << std::right << p.pid;

		std::cout.width(20);
		std::cout << std::left << " " + p.name;

		std::cout.width(20);
		std::cout << std::left << p.user;

		std::cout.width(18);
		std::cout << std::left << p.memV/1024;

		std::cout.width(14);
		std::cout << std::left << p.memRSS/1024;

		std::cout.width(14);
		std::cout << std::left << p.swap/1024;

		std::cout.width(14);
		std::cout << std::left << p.major_fault;

		std::cout.width(14);
		std::cout << std::left << p.minor_fault << "\n";
		count++;
	}
	
}

int main(int argc, char const *argv[]) {
	int targetComp = 0;
	int targetNumProc = 0;

	if (argc > 1) {
		std::string mi_string = argv[1];
		std::transform(mi_string.begin(), mi_string.end(), mi_string.begin(), ::tolower); // to lowercase
		if (mi_string.compare("pid") == 0)
		{
			targetComp = 1;
		} else if (mi_string.compare("virtual")  == 0) {
			targetComp = 2;

		} else if (mi_string.compare("swap")  == 0) {
			targetComp = 3;

		} else if (mi_string.compare("major")  == 0) {
			targetComp = 4;

		} else if (mi_string.compare("minor")  == 0) {
			targetComp = 5;
		} else if (mi_string.compare("rss")  == 0) {
			// RSS sort by default
		} else {
			std::cout << "Invalid parameter. Showing info sorted by RSS memory >>>\n";
			std::this_thread::sleep_for(std::chrono::seconds(2));	
		}
	} else {
		std::cout << "No parameter. Showing info sorted by RSS memory >>>\n";
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	if (argc > 2){
		std::string s_aux = argv[2];
		try {
			targetNumProc = stoi(s_aux);
		} catch (std::invalid_argument) {
			// Ignore. targetNumProc already set to 0
		}
	}

	std::vector<std::string> results;

	while(true) {
		system("clear");

		results = readDirectoryForProcesses("/proc");
		
		std::cout << "Total number of processes: " << results.size() << "\n";
		parseResults(results, targetComp, targetNumProc);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
