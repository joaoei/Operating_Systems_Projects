# Project 01
All assignments were done in the C++ language for the Operation Systems subject.
UFRN - 2017.

## Contributors
Gustavo Araújo Carvalho\s
Thiago César de Lucena

### Assignment 1.1: Form Bomb Prevent
This program creates a situation in which a fork bomb is dropped by an infinite loop inside the program 
itself, and then it tries to stop it when a number _n_ of processes created is reached. This prevents the machine to crash.
The limit of processes _n_ must be informed by the user. This number is stored in a shared memory area to 
be acessed by all forked processes. Once the total number of processes reach the limit, the program kills all processes and their group process, which is the process created when the program was fresh executed.

To compile, just insert the following command in the terminal ([filename] stands for the name you want to give to the executable):

	g++ -std=c++11 fork_bomb_final.cpp -o [filename]

Running the program can be done simply by running the executable itself, adding the limit _n_ of processes desired:

	./[executable] n

The output will show on the terminal the PID (process id) and PGID (process group ID) of the processes created. When the PGID process is killed, the message is also shown between process information lines.

### Assignment 1.2: Process List
This program builds a JSON file of the current active processes while also displaying some statistics and a simple tree view in the terminal.
This is done by reading and parsing the information stored in the /proc folder existent in the Unix-based systems.
To compile, just insert the following command in the terminal ([filename] stands for the name you want to give to the executable):

	g++ -std=c++11 process_list.cpp -o [filename]

Running the program can be done simply by running the executable itself:

	./[executable]

In this case, the program will start building the tree and JSON hierarchy from the process whose PID corresponds to 1. Typically this is the "init" process, it's the first to be created in the system after booting, and is the one that spawns most of the active process, so running the program with default parameters will generate the most information. Furthermore, the tree and the JSON file - named "stats.json" - will be updated every 5 seconds.

To finish the execution, type Ctrl+C.

Alternatively, you might want to run the program with

	./[executable] [PID]

In this case, the generated information will be only about the process specified as the second argument and its descendants.
