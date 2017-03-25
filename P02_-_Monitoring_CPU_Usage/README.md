# Project 01
All assignments were done in the C++ language for the Operational Systems subject.
UFRN - 2017.

## Contributors
Gustavo Araújo Carvalho
Thiago César de Lucena

### Assignment 2:
This program monitors the current cpu usage, printing its use percentage and lighting up leds to indicate how much of the
cpu is used. It also includes the function of optionally killing the most cpu expensive ess when the system is under
heavy operational load. It's meant to be used in a beaglebone black board.

It works by reading through all the current processes and reading their info inside /proc/PID/stat/, building up a
priority queue of the most cpu expensive ones, and showing the cpu usage to the user by green, yellow, and red led lights.
If the cpu usage is between 0% and 25%, only the green led lights up. Between 25% and 50% the yellow goes on too, and
between 50% and 75% the red also lights up. If the cpu usage is above 75%, the three lights will flicker and the button
will be enabled. Once pressed, the most costful process will be killed (hence the priority queue for organizing them)
and all lights will go off for 3 seconds, time in which the program will stay inactive until resuming its operation.

The green led uses the gpio51, the yellow gpio50 and the red gpio60. The button uses the gpio115.
Further instructions on how to set up the wires might come in the future.

To compile, just insert the following command in the beaglebone black terminal ([filename] stands for the name you want to give to the executable):

	g++ -std=c++11 main.cpp -gpio.cpp -o [filename]

Running the program can be done simply by running the executable itself:

	./[executable]
  
To stop the execution, press Ctrl+C

The files "killer.cpp" and "softkiller.cpp" are simple strain programs for testing the monitor. Just compile
and execute each of them separately just like specified above. soft_killer.cpp will put the cpu use to a little over 50%,
while killer.cpp will put the cpu use to 100%.
