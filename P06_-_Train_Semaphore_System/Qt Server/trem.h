#ifndef TREM_H
#define TREM_H

#include <QObject>
#include <thread>
#include <chrono>
#include <utility>
#include "point.h"
#include <QSemaphore>

using namespace std;

class Trem : public QObject
{
    Q_OBJECT
public:
    Trem(int, Point, Point, Point, Point, int, std::vector<QSemaphore*>&);
    ~Trem();
    void start();
    void run();
    void setVelocidade(int);
    bool getEnable();
    void setEnable(bool);
    bool isOnCritZone(const Point &, const int &);
    float calcStandardDeviation();

    static const int RC[10];
    static const std::pair<Point, Point> crZones[10];

signals:
    void updateGUI(int,int,int);
    void displayText(QString);
    void semaphoreChange(int, bool);
    void updateTime(int, float);
    void updateSD(int, float);
    void updateLastTime(int, float);

private:
   std::thread threadTrem;
   int id;
   Point position;
   Point topLeft;
   Point botRight;
   Point startPoint;
   std::vector<float> trips_time; //position 0 always stores the next position to be written

   std::chrono::time_point<std::chrono::system_clock> stationTime;
   int criticalFlags;
   std::vector<QSemaphore*>& sems;
   int currentRCZone;
   int velocidade;
   bool enable;
};

#endif // TREM_H



