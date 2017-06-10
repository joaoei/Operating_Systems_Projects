# Project 04
All assignments were done in the C++ programming language for the Operational Systems subject.
UFRN - 2017.

## Contributors
Gustavo Araújo Carvalho

Thiago César de Lucena

## Assignment 6:
This project is intended to implement a solution for the synchronization of urban trains using semaphores. Qt Creator IDE is used to model the rail network and run it as a server, and a Beaglebone Black is used as a terminal-based train controller to be runned as a client. The rail network server and train controller client communicate through sockets. 

### Rail Network server
The rail network server uses semaphores implementations to guarantee the access of only one train on critical zones, which are the paths on the trip of possible collision between trains. The rail network is simulated according to the image below.

![rail network](https://github.com/thiagocesarm/Operating_Systems_Projects/blob/master/P06_-_Train_Semaphore_System/images/trains.png)

The complete trip of each train is stated as follows:

* Train 1 -  zone
* Train 2 -  zone
* Train 3 -  zone
* Train 4 -  zone
* Train 5 -  zone
* Train 6 -  zone
* Train 7 -  zone

The semaphores (critical zones) are stated as follows:

* Semaphore 0 - Between pink and light blue zones.
* Semaphore 1 - Between light blue and green zones.
* Semaphore 2 - Between green and blue zones.
* Semaphore 3 - Between light blue and yellow zones.
* Semaphore 4 - Between green and yellow zones.
* Semaphore 5 - Between yellow and orange zones.
* Semaphore 6 - Between yellow and purple zones.
* Semaphore 7 - Between pink and orange zones.
* Semaphore 8 - Between orange and purple zones.
* Semaphore 9 - Between purple and blue zones.

Each train has its own speed, which can be modified by the train controller client. Also, some information about each of the trains' trips are showed on screen: time of last trip, average time of last ten trips, and standard deviation.

###Train Controller client
The Beaglebone Black is used to run the train controller client. Via terminal-based menus, the user can connect to the server and pause/resume all train trips, pause/resume a single train trip and change the speed of a specific train.

### BeagleBone Black configuration:
To use the train controller the way it was intended to be on this project, some electronic pieces will 
be needed: a potentiometer and three push buttons, along with some wires to connect them.

The BeagleBone Black proper configuration can be seen on the image below:
![bbb configuration](https://github.com/thiagocesarm/Operating_Systems_Projects/blob/master/P06_-_Train_Semaphore_System/images/semaphore_bb.png)

#### Controls:
* The top and bottom buttons are used to select the option desired.
* The middle button is used to confirm option through the menus.
* The potentiometer is used to modify the speed of a specific train.

### Executing project and Prerequisites:  
First thing to do is to turn on and connect the Beaglebone Black on your machine so the port and IP adresses used on the files are correct. Next, before compiling and running the program, be sure to have the `/Beagle Client` folder copied to your BeagleBone Black and the Qt Creator opened with the project, and have the components set on the right configuration like stated above.

**Compiling and running the server**

Simply open the `\Qt Server` project on Qt Creator and run it.

**Compiling and running the train controller client**

On BBB, enter `/Beagle Client` folder and input the following

```bash
make
sudo ./exe
```