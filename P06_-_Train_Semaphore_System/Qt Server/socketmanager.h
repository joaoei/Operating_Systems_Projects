#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <vector>
#include <thread>
#include "trem.h"

struct Packet {
    bool toggle;
    int target;
    int changeSpeed;
};

class SocketManager {
public:
    SocketManager(const std::vector<Trem *> &_trens);
    ~SocketManager();
    void socketListener();
    void modifyTrain(Packet pac);

private:
    std::thread threadTrem;
    const std::vector<Trem*>& trens;
    bool trainsEnable;
};

#endif // SOCKETMANAGER_H
