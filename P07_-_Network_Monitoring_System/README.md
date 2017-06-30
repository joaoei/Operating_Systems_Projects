# Project 07
All assignments were done in the C++ programming language for the Operational Systems subject.
UFRN - 2017.

## Contributors
Gustavo Araújo Carvalho

Thiago César de Lucena

## Assignment 
This project intends to create a central hub server for managing the running processes of computers connected in a network. A server computer can see and kill running processes of client computers and also blacklist process names as well. Blacklisted processes cannot be ran by any computer in the network.

### The Server
The server can see the top 25 running processes (sorted by CPU use) in every client server at any given time, it can also order any of these processes to be killed or to blacklist them so no connected computer can run then either.
The server also keeps a history of all computers that have been connected to it, keeping their id, number of connection sessions and total time spent connected.

### The Clients
Each client computer will gather CPU and RAM info every second and send it to the server. Each client also keeps its own copy of the blacklist in order to instantly kill undesirable processes.

### Compiling:
To compile both the server and the client, just run the _make_ command in their respective folders.
Notice that you can change the default IP and Port by modifying them on _socketmanager.hpp_ and _socketclient.hpp_.
```bash
make
```
### Running:
To run the executables, just execute them normally as:
```bash
./server
```
and
```bash
./client
```
From then on, follow the instructions on the server screen to execute actions.
Notice that the client won't be able to connect if there's no server running, so it's advised to run the server first.
