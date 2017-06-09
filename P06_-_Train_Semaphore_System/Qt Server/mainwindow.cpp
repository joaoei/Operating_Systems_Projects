#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for (int i = 0; i < 10; i++)
        semaphore.push_back(new QSemaphore(1));


    //
    trens.push_back(new Trem(1, Point(10,170), Point(10,20), Point(150,320), Point(10, 170), Trem::RC[0] | Trem::RC[7], semaphore));
    trens.push_back(new Trem(2, Point(220,20), Point(150,20), Point(290,120), Point(220, 20), Trem::RC[0] | Trem::RC[1] | Trem::RC[4], semaphore));
    trens.push_back(new Trem(3, Point(360,20), Point(290,20), Point(430,120), Point(360, 20), Trem::RC[1] | Trem::RC[2] | Trem::RC[3], semaphore));
    trens.push_back(new Trem(4, Point(570,170), Point(430,20), Point(570,320), Point(570, 170), Trem::RC[2] | Trem::RC[9], semaphore));
    trens.push_back(new Trem(5, Point(360,320), Point(290,220), Point(430,320), Point(360, 320), Trem::RC[6] | Trem::RC[8] | Trem::RC[9], semaphore));
    trens.push_back(new Trem(6, Point(220,320), Point(150,220), Point(290,320), Point(220, 320), Trem::RC[5] | Trem::RC[7] | Trem::RC[8], semaphore));
    trens.push_back(new Trem(7, Point(360,170), Point(220,120), Point(360,220), Point(360, 170), Trem::RC[3] | Trem::RC[4] | Trem::RC[5] | Trem::RC[6], semaphore));

    manager = new SocketManager(trens);

    for (auto i = 0u; i < trens.size(); i++) {
        connect(trens[i], SIGNAL(updateGUI(int,int,int)), SLOT(updateInterface(int,int,int)));
        connect(trens[i], SIGNAL(displayText(QString)), SLOT(updateText(QString)));
        connect(trens[i], SIGNAL(semaphoreChange(int, bool)), SLOT(updateSemLabel(int, bool)));
        connect(trens[i], SIGNAL(updateTime(int, float)), SLOT(updateTimeBox(int, float)));
        connect(trens[i], SIGNAL(updateSD(int, float)), SLOT(updateSDBox(int, float)));
        connect(trens[i], SIGNAL(updateLastTime(int, float)), SLOT(updateLastTimeBox(int, float)));
        trens[i]->start();
    }
}

MainWindow::~MainWindow() {
    for (auto pt : semaphore)
        delete pt;
    for (auto pt : trens)
        delete pt;
    delete manager;
    delete ui;
}

void MainWindow::updateInterface(int id, int x, int y) {
    switch (id) {
    case 1:
        ui->labelTrem01->setGeometry(x,y,20,20);
        break;
    case 2:
        ui->labelTrem02->setGeometry(x,y,20,20);
        break;
    case 3:
        ui->labelTrem03->setGeometry(x,y,20,20);
        break;
    case 4:
        ui->labelTrem04->setGeometry(x,y,20,20);
        break;
    case 5:
        ui->labelTrem05->setGeometry(x,y,20,20);
        break;
    case 6:
        ui->labelTrem06->setGeometry(x,y,20,20);
        break;
    case 7:
        ui->labelTrem07->setGeometry(x,y,20,20);
        break;
    }
}

void MainWindow::updateText(QString str) {
    auto text = ui->textEdit->toPlainText();
    ui->textEdit->setText(str + text);
}

void MainWindow::updateSemLabel(int id, bool free) {
    QString css;
    if (free)
        css = "background-color: rgb(115, 210, 22);";
    else
        css = "background-color: rgb(239, 41, 41);";

    switch (id) {
    case 0:
        ui->labelSem00->setStyleSheet(css);
        break;
    case 1:
        ui->labelSem01->setStyleSheet(css);
        break;
    case 2:
        ui->labelSem02->setStyleSheet(css);
        break;
    case 3:
        ui->labelSem03->setStyleSheet(css);
        break;
    case 4:
        ui->labelSem04->setStyleSheet(css);
        break;
    case 5:
        ui->labelSem05->setStyleSheet(css);
        break;
    case 6:
        ui->labelSem06->setStyleSheet(css);
        break;
    case 7:
        ui->labelSem07->setStyleSheet(css);
        break;
    case 8:
        ui->labelSem08->setStyleSheet(css);
        break;
    case 9:
        ui->labelSem09->setStyleSheet(css);
        break;
    }
}

void MainWindow::updateTimeBox(int id, float time) {
    QString mi_text = QString::number(time, 'f', 2);
    switch (id) {
    case 1:
        ui->boxTremTime1->setText(mi_text);
        break;
    case 2:
        ui->boxTremTime2->setText(mi_text);
        break;
    case 3:
        ui->boxTremTime3->setText(mi_text);
        break;
    case 4:
        ui->boxTremTime4->setText(mi_text);
        break;
    case 5:
        ui->boxTremTime5->setText(mi_text);
        break;
    case 6:
        ui->boxTremTime6->setText(mi_text);
        break;
    case 7:
        ui->boxTremTime7->setText(mi_text);
        break;
    }
}

void MainWindow::updateLastTimeBox(int id, float time) {
    QString mi_text = QString::number(time, 'f', 2);
    switch (id) {
    case 1:
        ui->boxTremLastTime1->setText(mi_text);
        break;
    case 2:
        ui->boxTremLastTime2->setText(mi_text);
        break;
    case 3:
        ui->boxTremLastTime3->setText(mi_text);
        break;
    case 4:
        ui->boxTremLastTime4->setText(mi_text);
        break;
    case 5:
        ui->boxTremLastTime5->setText(mi_text);
        break;
    case 6:
        ui->boxTremLastTime6->setText(mi_text);
        break;
    case 7:
        ui->boxTremLastTime7->setText(mi_text);
        break;
    }
}

void MainWindow::updateSDBox(int id, float time) {
    QString mi_text = QString::number(time, 'f', 2);
    switch (id) {
    case 1:
        ui->boxTremSD1->setText(mi_text);
        break;
    case 2:
        ui->boxTremSD2->setText(mi_text);
        break;
    case 3:
        ui->boxTremSD3->setText(mi_text);
        break;
    case 4:
        ui->boxTremSD4->setText(mi_text);
        break;
    case 5:
        ui->boxTremSD5->setText(mi_text);
        break;
    case 6:
        ui->boxTremSD6->setText(mi_text);
        break;
    case 7:
        ui->boxTremSD7->setText(mi_text);
        break;
    }
}

bool active = false;

void MainWindow::on_pushButton_clicked() {
    active = !active;
    for (int i = 0; i < 7; ++i) {
        trens[i]->setEnable(active);
    }
}
