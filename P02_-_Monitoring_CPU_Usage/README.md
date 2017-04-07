# Project 02
All assignments were done in the C++ language for the Operational Systems subject.
UFRN - 2017.

## Contributors
Gustavo Araújo Carvalho

Thiago César de Lucena

### Assignment 2:
This program monitors the current CPU usage, printing its use percentage and lighting up leds to indicate how much of the
CPU is being used. It also includes the function of optionally killing the most CPU expensive process when the system is under
heavy operational load. It's meant to be used in a BeagleBone Black board.

It works by going through all the current processes and reading their info inside /proc/PID/stat/, building up a
priority queue of the most CPU-expensive ones, and indicating the CPU usage to the user by green, yellow, and red LED lights.
If the CPU usage is between 0% and 25%, only the green led lights up. Between 25% and 50% the yellow goes on too, and
between 50% and 75% the red also lights up. If the CPU usage is above 75%, the three lights will flicker and the killing button
will be enabled. Once pressed, the most costful process will be killed (hence the priority queue for organizing them)
and all lights will go off for 3 seconds, time in which the program will stay inactive until resuming its operation.

The green LED uses the GPIO_51, the yellow one uses GPIO_50, and the red one uses GPIO_60. The button uses the GPIO_115.
Further instructions on how to set up the wires might come in the future.

To compile, just insert the following command in the BeagleBone Black terminal ([filename] stands for the name you want to give to the executable):

	g++ -std=c++11 main.cpp gpio.cpp -o [filename]

Running the program can be done simply by running the executable itself (must execute in super user mode):

	sudo ./[executable]
  
To stop the execution, press Ctrl+C.

The files "killer.cpp" and "soft_killer.cpp" are simple strain programs for testing the monitor and visualizing the LEDs functioning properly. Just compile
and execute each of them separately just like specified above. soft_killer.cpp will put the CPU use to a little over 50%,
while killer.cpp will put the CPU use near to 100%. Be sure to have the monitor program already running before executing the strain programs.
