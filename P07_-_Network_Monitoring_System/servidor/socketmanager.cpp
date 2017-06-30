#include "socketmanager.hpp"
#include "structs.hpp"

#include <cstdio>       //printf
#include <cstring>      //memset
#include <cstdlib>      //exit
#include <netinet/in.h> //htons
#include <arpa/inet.h>  //inet_addr
#include <sys/socket.h> //socket
#include <unistd.h>     //close
#include <thread>
#include <cstring>
#include <fstream>
#include <iomanip> //get_time
#include <mutex>
#include <chrono>

extern std::mutex connectionMutex;
extern std::mutex blacklistMutex;

SocketManager::SocketManager(std::list<ConnectionInfo> &_connected,
                             std::set<std::string> &_blacklist,
                             std::map<unsigned int, ClientInfo> &_history) :
                             connected(_connected), blacklist(_blacklist),
                             history(_history) {
    blNumOpDone = 0;
    blNextOpId = 1;
    
    // read history from file
    std::ifstream historyfile("server_history.txt");
    int i = 0;
    std::string buffer;
    while(historyfile.is_open() and !historyfile.eof()){
        ClientInfo mclient;
        
        historyfile >> mclient.id;
        historyfile >> mclient.numberTimesConnected;
        historyfile >> mclient.totalConnTime;
        
        if (mclient.id == 0) break;

        this->history.insert(std::pair<unsigned int, ClientInfo>(mclient.id, mclient));
        i++;
    }
    historyfile.close();
    
    // Update ID of next connection
    this->nextConnId = i + 1;
    
    // read blacklist from file
    std::ifstream blacklistfile("server_blacklist.txt");
    while(blacklistfile.is_open() and !blacklistfile.eof()){
        blacklistfile >> buffer;
        this->blacklist.insert(buffer);
    }
    blacklistfile.close();

    managerThread = std::thread(&SocketManager::waitConnections, this);
}

SocketManager::~SocketManager() {
    managerThread.detach();
    for (auto &th : connectionThreads) {
        th.detach();
    }

    connectionMutex.lock();
    blacklistMutex.lock();
    
    // write history to file
    if (!history.empty()) {
        std::cout << "Saving history...\n";
        std::ofstream historyfile("server_history.txt");
        if(historyfile.is_open()) {
            for(auto elem : this->history){
                historyfile << elem.second.id << "\n";
                historyfile << elem.second.numberTimesConnected << "\n";
                historyfile << elem.second.totalConnTime << "\n\n";
            }
        }
        historyfile.close();
    }

    // write blacklist to file
    if (!blacklist.empty()) {
        std::cout << "Saving blacklist...\n";
        std::ofstream blacklistfile("server_blacklist.txt");
        if(blacklistfile.is_open()){
            for (auto item : this->blacklist) {
                blacklistfile << item;
                blacklistfile << "\n";
            }
        }
        blacklistfile.close();
    } else {
        std::ifstream blacklistfile("server_blacklist.txt");
        if (blacklistfile.is_open())
            std::remove("server_blacklist.txt");
    }

    blacklistMutex.unlock();
    connectionMutex.unlock();
}

// Futuramente receberá um struct informando qual trem deverá ser modificado e qual a nova velocidade
void SocketManager::waitConnections() {
    //variáveis do servidor
    struct sockaddr_in endereco;
    int socketId;

    //variáveis relacionadas com as conexões clientes
    struct sockaddr_in sender;
    socklen_t sendersize = sizeof(sender);
    int conexaoClienteId;

    /*
     * Configurações do endereço
    */
    memset(&endereco, 0, sizeof(endereco));
    endereco.sin_family = AF_INET;
    endereco.sin_port = htons(PORTNUM);
    endereco.sin_addr.s_addr = inet_addr(IP);

    /*
     * Criando o Socket
     *
     * PARAM1: AF_INET ou AF_INET6 (IPV4 ou IPV6)
     * PARAM2: SOCK_STREAM ou SOCK_DGRAM
     * PARAM3: protocolo (IP, UDP, TCP, etc). Valor 0 escolhe automaticamente
    */
    socketId = socket(AF_INET, SOCK_STREAM, 0);

    //Verificar erros
    if (socketId == -1)
    {
        std::cout << "Failed in executing socket()\n";
        exit(EXIT_FAILURE);
    }

    //Conectando o socket a uma porta. Executado apenas no lado servidor
    if ( bind (socketId, (struct sockaddr *)&endereco, sizeof(struct sockaddr)) == -1 )
    {
        std::cout << "Failed in executing bind()\n";
        exit(EXIT_FAILURE);
    }

    //Habilitando o servidor a receber conexoes do cliente
    if ( listen( socketId, 10 ) == -1) {
        std::cout << "Failed in executing listen()\n";
        exit(EXIT_FAILURE);
    }

    int TEMP = 0;
    //servidor ficar em um loop infinito
    while(true) {
        conexaoClienteId = accept(socketId, (struct sockaddr *) &sender, &sendersize);

        if (conexaoClienteId == -1) {
            printf("Failed in executing accept()");
            exit(EXIT_FAILURE);
        }

        ConnectionInfo conInfo;

        /* Initializing connection */
        // <= get id 
        recv(conexaoClienteId, &conInfo.id, sizeof(int), 0);
        // if no id
        if (conInfo.id == 0) {
            // => send id
            conInfo.id = nextConnId;

            ClientInfo clInfo;
            clInfo.id = nextConnId;
            history.insert({nextConnId, clInfo});
            send(conexaoClienteId, &nextConnId, sizeof(int), 0);
            nextConnId++;
        }

        // => send blacklist

        blacklistMutex.lock();
        unsigned int blacklistSize = blacklist.size();
        send(conexaoClienteId, &blacklistSize, sizeof(unsigned int), 0);
        
        if (!blacklist.empty()) {
            char blacklistChar[blacklistSize][16];
            memset(blacklistChar, '\0', blacklist.size()*16);

            int i = 0;
            for (auto &str : blacklist)
                strcpy(blacklistChar[i++], str.c_str());

            send(conexaoClienteId, blacklistChar, sizeof(blacklistChar), 0);
        }
        blacklistMutex.unlock();

        // set initial time
        conInfo.connectedSince = time(NULL);
        conInfo.isAlive = true;

        // update history
        connectionMutex.lock();
        auto hisIt = history.find(conInfo.id);
        hisIt->second.numberTimesConnected++;

        killSchedule.insert({conInfo.id, std::list<int>()});

        connected.push_back(conInfo);
        connectionThreads.emplace_back(std::move(std::thread(&SocketManager::monitorConnection, this, conexaoClienteId, conInfo.id)));
        connectionMutex.unlock();
    }
}

void SocketManager::monitorConnection(const int conexaoClienteId, const int connId) {
	auto itConnected = --connected.end();
    auto itConnThreads = --connectionThreads.end();

    int byteslidos = 1;
	Packet pacoteRecebido;
    auto itToKillList = killSchedule.find(connId);
    
    while (true) {
        byteslidos = recv(conexaoClienteId, &pacoteRecebido, sizeof(pacoteRecebido), 0);
        if (byteslidos <= 0) break;
        // std::cout << byteslidos << "\n";
        // std::cout << "Mensagem recebida: " << std::string(pacote.info) << "\n";

        bool hasToKill = !itToKillList->second.empty();
        bool hasBlOperation = (!blacklistOperationQueue.empty() and
        blacklistOperationQueue.front().opId > itConnected->lastBlacklistOperation);

        char confirmationChar;
        if (hasToKill or hasBlOperation) {
            // send 1
            confirmationChar = 1;
            send(conexaoClienteId, &confirmationChar, sizeof(char), 0);

            OrderPacket order;
            order.lastPacket = !hasToKill;

            if (hasBlOperation) {
                // send blacklistOperation with !hasToKill as "last packet"
                if (blacklistOperationQueue.front().inserting)
                    order.operation = 2;
                else
                    order.operation = 3;
                strcpy(order.processName, blacklistOperationQueue.front().processName.c_str());
                send(conexaoClienteId, &order, sizeof(order), 0);

                itConnected->lastBlacklistOperation = blacklistOperationQueue.front().opId;
                blNumOpDone++;
                if (blNumOpDone == connected.size()) {
                    blacklistOperationQueue.pop_front();
                    blNumOpDone = 0;
                }
            }

            order.operation = 1;
            // lastPacket is already 0 if there's need to be sent

            while (itToKillList->second.size() > 1) {
                order.pid = itToKillList->second.front();
                itToKillList->second.pop_front();
                send(conexaoClienteId, &order, sizeof(order), 0);
            }

            order.lastPacket = true;

            if (hasToKill) {
                order.pid = itToKillList->second.front();
                itToKillList->second.pop_front();
                send(conexaoClienteId, &order, sizeof(order), 0);
            }
        } else {
            confirmationChar = 0;
            send(conexaoClienteId, &confirmationChar, sizeof(char), 0);
        }

        connectionMutex.lock();
        // update package connected
        itConnected->packet = pacoteRecebido;
        connectionMutex.unlock();
    }

    connectionMutex.lock();
    // update history
    history.find(connId)->second.totalConnTime += time(NULL) - itConnected->connectedSince;
    // delete packet of connected info
    itConnected->isAlive = false;
    connected.erase(itConnected);
    // delete thread of connectionThreads
    itConnThreads->detach();
    connectionThreads.erase(itConnThreads);
    killSchedule.erase(connId);
    
    connectionMutex.unlock();
    
    close(conexaoClienteId);
}

void SocketManager::scheduleKill(int id, int pid) {
    killSchedule.find(id)->second.push_back(pid);
}

void SocketManager::blacklistProcess(std::string procName) {
    if (procName.size() > 16)
        procName = procName.substr(0,16);

    blacklistMutex.lock();
    blacklist.insert(procName);
    blacklistMutex.unlock();
    
    BlacklistOperation blOp;
    blOp.inserting = true;
    blOp.processName = procName;
    blOp.opId = blNextOpId++;
    blacklistOperationQueue.push_back(blOp);
}

void SocketManager::removeBlacklist(int index) {
    auto it = blacklist.begin();
    for (int i = 0; i < index and it != blacklist.end(); i++) it++;

    if (it == blacklist.end())
        return;

    BlacklistOperation blOp;
    blOp.inserting = false;
    blOp.processName = *it;
    blOp.opId = blNextOpId++;
    blacklistOperationQueue.push_back(blOp);

    blacklistMutex.lock();
    blacklist.erase(it);
    blacklistMutex.unlock();
}