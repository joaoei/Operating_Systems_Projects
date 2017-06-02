# Project 05
All assignments were done in the C++ language for the Operational Systems subject.
UFRN - 2017.

## Contributors
Gustavo Araújo Carvalho

Thiago César de Lucena

### Assignment:
This program monitors the current memory used by the system, showing information about main memory, cache memory and swap consumption in general. Furthermore, it provides information about virtual and resident (RSS) memories and swap space about each of the executing processes, along with their amount of page faults.

All processes are showed on terminal as a list sorted by RSS memory use, as a default sorting parameter. The user may change the sorting parameter and number of processes to be displayed.

The program works by going through all the current processes and reading their info inside /proc/PID/status/ and /proc/PID/stat/, building up a multiset of the most memory-expensive ones according to the parameter chose. The general information about the system's memory is obtained at /proc/meminfo.

To compile, just insert the following command in the terminal ([executable] stands for the name you want to give to the executable):

	g++ -std=c++11 main.cpp -o [executable]

Running the program can be done simply by running the executable itself along with the parameters desired:

	./[executable] [sort option] [processes amount]

[sort option] The sorting parameter. May be empty. Options are:
- pid : Sort by PID number.
- virtual : Sort by virtual memory size.
- rss : Sort by resident memory size.
- swap : Sort by swap consumed.
- major : Sort by major page faults.
- minor : Sort by minor page faults.

[processes amount] The amount of processes to be displayed. May be empty. If set, must be after [sort option].
  
For proper visualisation, run it with the terminal window maximized. To stop the execution, press Ctrl+C.
