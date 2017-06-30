#pragma once

#include <iostream>
#include <list>
#include <set>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <cstdlib>
#include <mutex>
#include <iomanip>
#include "structs.hpp"
#include "socketmanager.hpp"
#include "buffertoggle.hpp"

class Application {
public:
	Application() : connected(), blacklist(), history(),
					socketmanager(connected, blacklist, history),
				    currentMenu(MAIN_MENU), keepGoing(true) {
		buffer.off();
	}
	
	void launch();

private:
	std::list<ConnectionInfo> connected;
	std::set<std::string> blacklist;
    std::map<unsigned int, ClientInfo> history;
	BufferToggle buffer;
	SocketManager socketmanager;
	int currentMenu;
	bool keepGoing;

	enum MENUS {
		MAIN_MENU,
		LIST_CONNECTED,
		LIST_CONNECTED_PROCESSES,
		CONNECTED_HISTORY,
		HISTORY_STATS,
		SHOW_PROCESS_BLACKLIST,
		END
	};

	void getCharNoBuffer(char &c);
	void getInput(std::string &str, bool &isDone);

	void showMainMenu();
	int showListConnected();
	void showConnectedProcesses(int id);

	void showConnectedHistory();

	void showBlacklist();
};
