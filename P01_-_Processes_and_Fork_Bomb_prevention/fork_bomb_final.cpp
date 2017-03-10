#include <unistd.h> // fork()
#include <iostream>
#include <signal.h> // kil()
#include <sys/mman.h> // mmap()

static int * process_counter; // Total number of processes supported

int main(int argc, char *argv[]){
    printf("Starting program.\n");

    // Few arguments
    if(argc < 2){
    	std::cout << "Please input the number of processes supported by program.\n";
    	return 0;
    }

    int input_number = atoi(argv[1]); // Input number of processes

    if(input_number < 1){
    	std::cout << "Invalid total number of processes.\n";
    	return 0;
    }

    // Shared memory variable (function mmap())
    process_counter = (int *) mmap(0, sizeof *process_counter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    *process_counter = 1;

	while(true){
		printf(" ---> PID: %d, PGID: %d \n",getpid(), getpgid(getpid()));
		if (*process_counter > input_number)
		{
            kill(getpgid(getpid()), SIGKILL); // Kills the process group ID (PGID) of current process
			kill(getpid(), SIGKILL); // Kills current process
		}
		*process_counter = *process_counter + 1;
		fork();
	}

	return 0;
};