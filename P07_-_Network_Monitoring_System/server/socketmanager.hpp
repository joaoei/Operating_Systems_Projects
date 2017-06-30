#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include <set>
#include <thread>
#include <unordered_map>
#include "structs.hpp"

#define PORTNUM 11112
#define IP "127.0.0.1"

class SocketManager {
public:
    SocketManager(std::list<ConnectionInfo> &_connected,
                  std::set<std::string> &_blacklist,
                  std::map<unsigned int, ClientInfo> &_history);
    ~SocketManager();
    void waitConnections();
    void monitorConnection(const int conexaoClienteId, const int connId);
    void scheduleKill(int connectionIndex, int pid);
    void blacklistProcess(std::string procName);
    void removeBlacklist(int index);

private:
    std::thread managerThread;
    std::list<ConnectionInfo> &connected;
    std::set<std::string> &blacklist;
    std::map<unsigned int, ClientInfo> &history;

    int nextConnId;

    std::list<BlacklistOperation> blacklistOperationQueue;
    unsigned int blNextOpId;
    int blNumOpDone;

    std::list<std::thread> connectionThreads;
    std::unordered_map<int, std::list<int>> killSchedule; 
};

#endif // SOCKETMANAGER_H