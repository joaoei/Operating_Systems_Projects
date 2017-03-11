#include <cstdlib>
#include <vector>
#include <list>
#include <unordered_map>
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

// struct to store process info
struct Process {
	std::string name;
	std::string user;
	int pid;
	int ppid;
};

// function signatures
// ----------------------------------------------------------------------------
bool isdigit(const std::string& str);
std::vector<std::string> readDirectoryForProcesses(const std::string& path);
void parseResults(const std::vector<std::string> &processes);
void printTree(const std::unordered_map<int, std::list<Process>>& child_list, int currPPID, int base_ind);
std::string prepareJSON(const std::unordered_map<int, std::list<Process>>& child_list, int currPPID, int base_ind);
void writeJSON(const std::string &str);
std::string genSpaces(const int &space);
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

void parseResults(const std::vector<std::string>& processes, const int& targetPID) {
	char temp[1000];
	std::unordered_map<int, std::list<Process>> child_list;
	std::unordered_map<int, Process> process_map;
	std::unordered_map<std::string, int> user_process_count;
	for(const auto& proc : processes) {
		std::string filename("/proc/" + proc + "/status");
		std::ifstream fs(filename);
		Process p;

		// Name
		fs.getline(temp, 1000);
		std::string sName(temp);
		p.name = sName.substr(6);

		// user
		struct stat info;
		stat(filename.c_str(), &info);
		struct passwd *pw = getpwuid(info.st_uid);
		p.user = std::string(pw->pw_name);

		auto itUser = user_process_count.find(p.user);
		if (itUser == user_process_count.end())
			user_process_count.insert({p.user, 1});
		else
			itUser->second++;

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

		// Inserting on maps
		process_map.insert({p.pid, p});

		auto it_child_list = child_list.find(p.ppid);
		if (it_child_list == child_list.end()) {
			std::list<Process> temp_list;
			temp_list.push_back(p);
			child_list.insert({p.ppid, temp_list});
		} else {
			it_child_list->second.push_back(p);
		}

		fs.close();
	}

	// Processes of each user
	for(const auto& a : user_process_count) {
		std::cout << "Numero de processos pertencentes a " << a.first
		          << ": " << a.second << "\n";
	}
	std::cout << "\n";

	// Terminal tree
	auto itInitialProcess = process_map.find(targetPID);
	if (itInitialProcess != process_map.end()) {
		Process& p = itInitialProcess->second;

		std::cout << p.name << "\n";
		printTree(child_list, p.pid, 2);

		// JSON
		std::string json = "{\n  \"Name\": \"" + p.name
			      + "\",\n  \"User\": \"" + p.user
				  + "\",\n  \"PID\": " + std::to_string(p.pid);
			
		std::string tempres = prepareJSON(child_list, p.pid, 4);
		if (tempres != "")
			json += ",\n  \"Processos filhos\": [\n"
			+ tempres + "  \n  ]";

		json += "\n}";

		writeJSON(json);
	} else {
		std::cout << "O processo especificado não existe\n";
	}
}

// Recursively prints the process tree on the terminal
void printTree(const std::unordered_map<int, std::list<Process>>& child_list,
			   int currPPID, int base_ind) {
	auto it = child_list.find(currPPID);
	if (it == child_list.end())
		return;

	const std::list<Process>& list_alias = it->second;
	for(const auto& p : list_alias) {
		std::cout << genSpaces(base_ind) << p.name << "\n";
		printTree(child_list, p.pid, base_ind + 2);
	}
}

// Recursively builds the JSON data
std::string prepareJSON(const std::unordered_map<int, std::list<Process>>& child_list,
						int currPPID = 0, int base_ind = 0) {
	std::string final, tempres;
	auto it = child_list.find(currPPID);
	if (it == child_list.end())
		return final;

	const std::list<Process>& list_alias = it->second;
	bool use_comma = false;

	for(const auto& p : list_alias) {
		if (use_comma) final += ",\n";
		final += genSpaces(base_ind) + "{\n" + genSpaces(base_ind + 2) + "\"Name\": \"" + p.name
		      + "\",\n" + genSpaces(base_ind + 2) + "\"User\": \"" + p.user
			  + "\",\n" + genSpaces(base_ind + 2) + "\"PID\": " + std::to_string(p.pid);
		
		tempres = prepareJSON(child_list, p.pid, base_ind + 4);
		if (tempres != "")
			final += ",\n" + genSpaces(base_ind + 2) + "\"Processos filhos\": [\n"
			+ tempres + genSpaces(base_ind + 2) + "\n" + genSpaces(base_ind + 2) + "]";

		final += "\n" + genSpaces(base_ind) + "}";
		use_comma = true;
	}

	return final;
}

// Writes JSON to file
void writeJSON(const std::string &str) {
	std::ofstream os("stats.json");
	os << str;
	os.close();
}

// Generates blankspace
std::string genSpaces(const int &space) {
	return std::string(space, ' ');
}

int main(int argc, char const *argv[]) {
	int targetPID = 1;
	if (argc > 1) {
		targetPID = std::stoi(argv[1]);
		std::cout << ">>> Mostrando informações do processo número " << targetPID << "...\n";
	} else {
		std::cout << ">>> Nenhum parametro, mostrando informações do processo init\n";
	}

	std::vector<std::string> results;

	while(true) {
		system("clear;clear;clear");

		results = readDirectoryForProcesses("/proc");
		
		std::cout << "Numero de processos: " << results.size() << "\n";
		parseResults(results, targetPID);
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
}
