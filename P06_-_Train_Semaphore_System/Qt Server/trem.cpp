#include "trem.h"
#include <iostream>
#include <sstream>
#include <QDebug>

// RC0
// (150), (20 -> 120)
// RC1
// (290), (20 -> 120)
// RC2
// (430), (20 -> 120)
// RC3
// (220-290), (120)
// RC4
// (360-430), (120)
// RC5
// (220-290), (220)
// RC6
// (360-430), (220)
// RC7
// (150), (220 -> 320)
// RC8
// (290), (220 -> 320)
// RC9
// (430), (220 -> 320)

const int Trem::RC[10] = {1, 1 << 1, 1 << 2, 1 << 3, 1 << 4, 1 << 5,
                    1 << 6, 1 << 7, 1 << 8, 1 << 9};

//const std::pair<Point, Point> Trem::crZones[10] = {
//        std::pair<Point, Point>(Point(150, 20), Point(150, 120)),
//        std::pair<Point, Point>(Point(290, 20), Point(290, 120)),
//        std::pair<Point, Point>(Point(430, 20), Point(430, 120)),
//        std::pair<Point, Point>(Point(220, 120), Point(360, 120)), // 4
//        std::pair<Point, Point>(Point(220, 120), Point(360, 120)), // 3
//        std::pair<Point, Point>(Point(220, 220), Point(360, 220)), // 5
//        std::pair<Point, Point>(Point(220, 220), Point(360, 220)), // 6
//        std::pair<Point, Point>(Point(150, 220), Point(150, 320)),
//        std::pair<Point, Point>(Point(290, 220), Point(290, 320)),
//        std::pair<Point, Point>(Point(430, 220), Point(430, 320))};

const std::pair<Point, Point> Trem::crZones[10] = {
        std::pair<Point, Point>(Point(140, 10), Point(160, 130)),
        std::pair<Point, Point>(Point(280, 10), Point(300, 130)),
        std::pair<Point, Point>(Point(420, 10), Point(440, 130)),
        std::pair<Point, Point>(Point(210, 110), Point(370, 130)), // 4
        std::pair<Point, Point>(Point(210, 110), Point(370, 130)), // 3
        std::pair<Point, Point>(Point(210, 210), Point(370, 230)), // 5
        std::pair<Point, Point>(Point(210, 210), Point(370, 230)), // 6
        std::pair<Point, Point>(Point(140, 210), Point(160, 330)),
        std::pair<Point, Point>(Point(280, 210), Point(300, 330)),
        std::pair<Point, Point>(Point(420, 210), Point(440, 330))};

Trem::Trem(int _id, Point _position, Point _topLeft, Point _botRight, Point _startPoint, int _crFlags, std::vector<QSemaphore*>& _semaphore) :
    id(_id), position(_position), topLeft(_topLeft), botRight(_botRight), startPoint(_startPoint), criticalFlags(_crFlags), sems(_semaphore) {
    currentRCZone = 0;
    velocidade = 250;
    enable = false;
    trips_time.push_back(1); //position 0 always stores the next position to be written
}

Trem::~Trem() {
    threadTrem.detach();
}

void Trem::setVelocidade(int velocidade) {
    this->velocidade = velocidade;
}

bool Trem::getEnable() {
    return enable;
}

void Trem::setEnable(bool enable) {
    this->enable = enable;
}

void Trem::start() {
    threadTrem = std::thread(&Trem::run,this);
}

bool Trem::isOnCritZone(const Point &pos, const int &zone) {
    return (pos.x >= crZones[zone].first.x &&
            pos.y >= crZones[zone].first.y &&
            pos.x <= crZones[zone].second.x &&
            pos.y <= crZones[zone].second.y);
}

void Trem::run() {
    Point temp;
    stationTime = std::chrono::system_clock::now();

    while(true) {
        if (enable) {
            temp = position;
            if (position.y == topLeft.y && position.x < botRight.x)
                temp.x += 10;
            else if (position.x == botRight.x && position.y < botRight.y)
                temp.y += 10;
            else if (position.x > topLeft.x && position.y == botRight.y)
                temp.x -= 10;
            else
                temp.y -= 10;

            // Entry check
            for (int i = 0; i < 10; ++i) {
                // Se  pode estar na ZC      e       está na ZC        e      não é dono da ZC
                if ((criticalFlags & RC[i]) and isOnCritZone(temp, i) and !(currentRCZone & RC[i])) {
                    // Vire dono
                    currentRCZone = currentRCZone | RC[i];
                    std::stringstream str;
                    str << id << ": tenta conseguir sem " << i << "\n";
                    emit displayText(QString(str.str().c_str()));

                    sems[i]->acquire();
                    emit semaphoreChange(i, false);

                    std::stringstream str2;
                    str2 << id << ": conseguiu sem  " << i << "\n";
                    emit displayText(QString(str2.str().c_str()));
                }
            }

            position = temp;
            emit updateGUI(id, position.x, position.y);

            // Time
            if (position == startPoint) {
                std::chrono::time_point<std::chrono::system_clock> temp = std::chrono::system_clock::now();
                float timeSecs = (std::chrono::duration_cast<std::chrono::milliseconds> (temp - stationTime).count())/1000.0;

                // when inserting first 10 elements on vector
                if(trips_time.size() < 11)
                    trips_time.push_back(timeSecs);
                else
                    trips_time[trips_time[0]] = timeSecs;

                // updating next position to store last trip time
                trips_time[0] = ((int)trips_time[0] % 10) + 1;
                float media = 0;
                for (unsigned int i = 1; i < trips_time.size(); ++i) {
                    media += trips_time[i];
                }
                media /= (float) (trips_time.size() - 1);

                emit updateTime(id, media);
                emit updateSD(id, calcStandardDeviation());
                emit updateLastTime(id, timeSecs);
                stationTime = temp;
            }

            for (int i = 0; i < 10; ++i) {
                // Se  pode estar na ZC    e     não está na ZC       e        é dono da ZC
                if (criticalFlags & RC[i] and !isOnCritZone(temp, i) and (currentRCZone & RC[i])) {
                    // Libere
                    currentRCZone = currentRCZone & (~RC[i]);
                    std::stringstream str;
                    str << id << ": tenta liberar sem " << i << "\n";
                    emit displayText(QString(str.str().c_str()));

                    sems[i]->release();
                    emit semaphoreChange(i, true);

                    std::stringstream str2;
                    str2 << id << ": liberou sem " << i << "\n";
                    emit displayText(QString(str2.str().c_str()));
                }
            }
        }


        this_thread::sleep_for(chrono::milliseconds(velocidade));
    }
}

float Trem::calcStandardDeviation() {
    if (trips_time.size() < 3)
        return 0;

    float oldM, m = 0, s = 0, x;
    for (unsigned int i = 1; i < trips_time.size(); i++) {
        x = trips_time[i];
        oldM = m;
        m = m + (x-m)/(float) i;
        s = s + (x-m)*(x-oldM);
    }

    return sqrt(s/(trips_time.size()-2));
}

