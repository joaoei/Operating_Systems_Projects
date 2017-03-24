// sleep
#include <chrono>
#include <thread>

int main(int argc, char const *argv[]) {
	while(true) {
		std::this_thread::sleep_for(std::chrono::nanoseconds(100));
	    int a = 1 + 2;
	}
	return 0;
}