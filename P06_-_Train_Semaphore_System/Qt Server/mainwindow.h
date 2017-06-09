#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "trem.h"
#include "point.h"
#include "socketmanager.h"
#include <QMainWindow>
#include <QSemaphore>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void updateInterface(int,int,int);
    void updateText(QString);
    void updateSemLabel(int, bool);
    void updateTimeBox(int, float);
    void updateSDBox(int, float);
    void updateLastTimeBox(int, float);

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<Trem*> trens;
    SocketManager *manager;
    std::vector<QSemaphore*> semaphore;
};

#endif // MAINWINDOW_H
