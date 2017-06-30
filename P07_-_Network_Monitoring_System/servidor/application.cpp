#include "application.hpp"

extern std::mutex connectionMutex;
extern std::mutex blacklistMutex;

std::string toLower(const std::string &in) {
	std::string ret = in;
	for (char &c : ret)
		if (c >= 'A' and c <= 'Z')
			c = (char) (c - 'A' + 'a');

	return ret;
}

void Application::launch() {
	int selectionIndex = 0;
	
	while (keepGoing) {
		system("clear");
	
		switch (currentMenu) {
		  case MAIN_MENU:
		  	showMainMenu();
			break;

		  case LIST_CONNECTED:
		  	selectionIndex = showListConnected();
			break;

		  case LIST_CONNECTED_PROCESSES:
		  	showConnectedProcesses(selectionIndex);
			break;

		  case CONNECTED_HISTORY:
		  	showConnectedHistory();
			break;

		  case SHOW_PROCESS_BLACKLIST:
		  	showBlacklist();
			break;

		  default:
		  	keepGoing = false;
			break;
		}
	}
}

void Application::getCharNoBuffer(char &c) {
	c = getchar();
}

void Application::getInput(std::string &str, bool &isDone) {
    char c = getchar();
    while (c != '\n') {
    	if ((int) c == 127 and !str.empty()) str.pop_back();
    	else str.push_back(c);
        c = getchar();
    }

    isDone = true;
}

void Application::showMainMenu() {
	std::cout << "Pick and press the number corresponding to any of the following options:\n"
			  << "1 - List of connected clients\n"
			  << "2 - History of connections\n"
			  << "3 - Process blacklist\n"
			  << "Press Q to quit.\n";
	char option = 0;
	getCharNoBuffer(option);

	switch (option) {
	  case '1':
	  	currentMenu = LIST_CONNECTED;
	  	break;
	  case '2':
	  	currentMenu = CONNECTED_HISTORY;
	  	break;
	  case '3':
	  	currentMenu = SHOW_PROCESS_BLACKLIST;
	  	break;

	  case 'Q':
	  case 'q':
	  	currentMenu = END;
	  	break;
	}
}

int Application::showListConnected() {
	std::string input;
	bool isDone = false;
	std::thread thr = std::thread(&Application::getInput, this, std::ref(input), std::ref(isDone));

	while (!isDone) {
		std::cout << "Choose an id or press Q to return:\n";
		
		std::cout << " ID | %CPU | TotalMEM | Session Time (seconds)\n";
		connectionMutex.lock();
		time_t currentTime = time(NULL);
		std::cout << std::setprecision(2) << std::fixed;
		for (auto &con : connected) {
			std::cout << " ";
			std::cout.width(4);
			std::cout << std::left << con.id;

			std::cout << " ";
			std::cout.width(6);
			std::cout << std::left << con.packet.totalCPUuse;

			std::cout << " ";
			std::cout.width(10);
			std::cout << std::left << con.packet.totalMEMuse;

			std::cout << " ";
			std::cout.width(24);
			std::cout << std::left << currentTime - con.connectedSince;

			std::cout << "\n";
		}
		connectionMutex.unlock();

		std::cout << "\n> " << input << "\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		system("clear");
	}

	thr.join();

	if (input == "Q" or input == "q")
		currentMenu = MAIN_MENU;
	else if (isdigit(input[0])) {
		currentMenu = LIST_CONNECTED_PROCESSES;
		return std::stoi(input);
	}

	return 0;
}

void Application::showConnectedProcesses(const int id) {
	connectionMutex.lock();
	auto itConn = connected.begin();
	
	while(itConn != connected.end() and itConn->id != id) ++itConn;

	connectionMutex.unlock();

	if (itConn == connected.end()) {
		currentMenu = LIST_CONNECTED;
		return;
	}

	std::string input;
	bool isDone = false;
	std::thread thr = std::thread(&Application::getInput, this, std::ref(input), std::ref(isDone));

	while (!isDone) {
		std::cout << "K [PID] to kill process, BL [process name] to blacklist process, Q to return\n";
		connectionMutex.lock();
		if (!itConn->isAlive) {
			input = "Q";
			break;
		}
		
		Packet &pack = itConn->packet;
		std::cout << " PID |   Process Name   | %CPU | memV | memRSS\n";
		std::cout << std::setprecision(2) << std::fixed;
		for (int i = 0; i < 25; ++i) {
			std::cout << " ";
			std::cout.width(5);
			std::cout << std::left << pack.topProcesses[i].pid;

			std::cout << " ";
			std::cout.width(18);
			std::cout << std::left << std::string(pack.topProcesses[i].name);

			std::cout << " ";
			std::cout.width(6);
			std::cout << std::left << pack.topProcesses[i].cpuUsagePercent;

			std::cout << " ";
			std::cout.width(6);
			std::cout << std::left << pack.topProcesses[i].memV;

			std::cout << " ";
			std::cout.width(6);
			std::cout << std::left << pack.topProcesses[i].memRSS;

			std::cout << "\n";
		}
		connectionMutex.unlock();

		std::cout << "\n> " << input << "\n";
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		system("clear");
	}

	isDone = true;
	thr.join();

	if (input == "Q" or input == "q")
		currentMenu = LIST_CONNECTED;
	else if (input.size() > 2 and toLower(input.substr(0,1)) == "k") {
		std::string substr = input.substr(2);
		if (isdigit(substr[0]))
			socketmanager.scheduleKill(id, std::stoi(substr));
	} else if (input.size() > 3 and toLower(input.substr(0,2)) == "bl") {
		std::string substr = input.substr(3);
		socketmanager.blacklistProcess(substr);
	}
}

void Application::showConnectedHistory() {
	std::cout << "Press Q to return:\n";
	std::cout << " ID | No of Sessions | Total Past Sessions Time\n";
	std::cout << std::setprecision(2) << std::fixed;
	for (auto &val : history) {
		std::cout << " ";
		std::cout.width(4);
		std::cout << std::left << val.second.id;

		std::cout << " ";
		std::cout.width(16);
		std::cout << std::left << val.second.numberTimesConnected;

		std::cout << " ";
		std::cout.width(24);
		std::cout << std::left << val.second.totalConnTime;

		std::cout << "\n";
	}

	std::cout << "\n> ";
	char option;
	getCharNoBuffer(option);

	if (option == 'Q' or option == 'q')
		currentMenu = MAIN_MENU;
}

void Application::showBlacklist() {
	std::cout << "add [process name] to add to blacklist, rem [index] to remove from blacklist, Q to return\n";
	
	int i = 0;
	std::cout << " Index | Process name\n";
	blacklistMutex.lock();
	std::cout << std::setprecision(2) << std::fixed;
	for (auto &str : blacklist){
		std::cout << " ";
		std::cout.width(7);
		std::cout << std::left << i++;

		std::cout << " ";
		std::cout.width(12);
		std::cout << std::left << str;

		std::cout << "\n";	
	}

	blacklistMutex.unlock();

	std::cout << "\n> ";
	std::string input;
	bool isDone;
	getInput(input, isDone);

	if (input == "Q" or input == "q")
		currentMenu = MAIN_MENU;
	else if (input.size() > 4) {
		if (toLower(input.substr(0,3)) == "add") {
			std::string substr = input.substr(4);
			socketmanager.blacklistProcess(substr);
		} else if (toLower(input.substr(0,3)) == "rem") {
			std::string substr = input.substr(4);
			if (isdigit(substr[0]) and std::stoi(substr) < blacklist.size())
				socketmanager.removeBlacklist(std::stoi(substr));
		}
	}
}