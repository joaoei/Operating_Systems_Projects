#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <cstdlib>

#include "adc.hpp"
#include "gpio.hpp"
#include "socketclient.hpp"

enum SCREENS {MAIN, TURN_ON_TR_SELECTOR, SPEED_TR_SELECTOR, SPEED_PICKER, END};

std::mutex potMutex;
std::mutex butMutex;

// Conectar / Desconectar
// Ligar/Desligar TODOS
// Ligar/Desligar ESPECIFICO
// Mudar velocidade

int mainMenu(SocketClient* &socketCl, unsigned int &menuSelection, bool &execute) {
	menuSelection %= 4;

    if (execute) {
    	execute = false;

    	int returnVal;
    	switch (menuSelection){
    	  case 0:
    		delete socketCl;
    		socketCl = nullptr;

    	  	returnVal = MAIN;
    	  break;
    	  case 1:
    	    socketCl->sendPacket(Packet(true, -1, 0));
    	  	returnVal = MAIN;
    	  break;
    	  case 2:
    	  	returnVal = TURN_ON_TR_SELECTOR;
    	  break;
    	  case 3:
    	  	returnVal = SPEED_TR_SELECTOR;
    	  break;
    	}

    	menuSelection = 0;
    	return returnVal;
    }

    if (menuSelection == 0) std::cout << "# "; else std::cout << "  ";
    std::cout << "Desconectar do servidor\n";
    if (menuSelection == 1) std::cout << "# "; else std::cout << "  ";
    std::cout << "Ligar/Desligar todos os trens\n";
    if (menuSelection == 2) std::cout << "# "; else std::cout << "  ";
    std::cout << "Ligar/Desligar um trem específico\n";
    if (menuSelection == 3) std::cout << "# "; else std::cout << "  ";
   	std::cout << "Alterar velocidade de um trem\n";

   	return MAIN;
}

int toggleTrainSelection(SocketClient *socketCl, unsigned int &menuSelection, bool &execute) {
	menuSelection %= 8;

    if (execute) {
    	execute = false;

    	int returnVal = TURN_ON_TR_SELECTOR;
    	if (menuSelection < 7)
    		socketCl->sendPacket(Packet(true, menuSelection, 0));
    	else
    		returnVal = MAIN;

    	menuSelection = 0;
    	return returnVal;
    }

    for (int i = 0; i < 7; ++i) {
    	if (menuSelection == i) std::cout << "# "; else std::cout << "  ";
    	std::cout << "Ligar/Desligar trem #" << i+1 << "\n";
    }
    if (menuSelection == 7) std::cout << "# "; else std::cout << "  ";
    std::cout << "Voltar\n";


   	return TURN_ON_TR_SELECTOR;
}

int modifyTrainSpeedSelection(unsigned int &menuSelection, bool &execute) {
	menuSelection %= 8;

    if (execute) {
    	execute = false;
    	
    	if (menuSelection < 7)
    		return SPEED_PICKER;
    	
    	menuSelection = 0;
    	return MAIN;
    }

    for (int i = 0; i < 7; ++i) {
    	if (menuSelection == i) std::cout << "# "; else std::cout << "  ";
    	std::cout << "Modificar a velocidade do trem #" << i+1 << "\n";
    }
    if (menuSelection == 7) std::cout << "# "; else std::cout << "  ";
    std::cout << "Voltar\n";


   	return SPEED_TR_SELECTOR;
}

int pickSpeed(SocketClient *socketCl, unsigned int &menuSelection, const int &pot, bool &execute) {
	if (execute) {
		execute = false;
		return SPEED_TR_SELECTOR;
	}

	std::cout << "## Modificando velocidade do trem #" << menuSelection+1 << "\n"
			  << "Pressione PLAY para voltar\n";

	potMutex.lock();
	socketCl->sendPacket(Packet(false, menuSelection, pot));
	potMutex.unlock();

	return SPEED_PICKER;
}

void updatePotenciometer(int &pot) {
	bbb::ADC adc_pot(0);

	while (true) {
		potMutex.lock();
		pot = adc_pot.getValue();
		potMutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(12));
	}
}

void updateButton(bool &button, const int& id) {
	bbb::GPIO gpio_but(id);
	gpio_but.setModeIn();
	
	while (true) {
		butMutex.lock();
		button = gpio_but.getValue();
		butMutex.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds(12));
	}
}

int main(int argc, char const *argv[]) {
	
	bool OLD_upButton, OLD_downButton, OLD_playButton,
		 upButton, downButton, playButton = false;
	OLD_upButton = OLD_downButton = OLD_playButton =
		upButton = downButton = playButton = false;

	int potenciometer = 0;

	int currentScreen = MAIN;
	unsigned int menuSelection = 0;
	bool executeAction = false;
	
	std::thread upButtonThread(updateButton, std::ref(upButton), 50);
	std::thread downButtonThread(updateButton, std::ref(downButton), 115);
	std::thread playButtonThread(updateButton, std::ref(playButton), 51);
	std::thread potenciometerThread(updatePotenciometer, std::ref(potenciometer));

	SocketClient *socketCl = nullptr;

	// menu
	while(currentScreen != END) {
		butMutex.lock();
		if (!OLD_upButton and upButton) menuSelection--;
		if (!OLD_downButton and downButton) menuSelection++;
		if (!OLD_playButton and playButton)	executeAction = true;
		OLD_upButton = upButton;
		OLD_downButton = downButton;
		OLD_playButton = playButton;
		butMutex.unlock();

		if (socketCl == nullptr) {
			menuSelection %= 2;

			if (executeAction) {
				if (menuSelection == 0)
					socketCl = new SocketClient();
				else
					currentScreen = END;
				executeAction = false;
			} else {
				if (menuSelection == 0) std::cout << "# "; else std::cout << "  ";
				std::cout << "Conectar ao servidor\n";
				if (menuSelection == 1) std::cout << "# "; else std::cout << "  ";
				std::cout << "Sair\n";
			}
		
		} else {
			switch (currentScreen) {
			  case MAIN:
		    	currentScreen = mainMenu(socketCl, menuSelection, executeAction);
			  break;
			  case TURN_ON_TR_SELECTOR:
			  	currentScreen = toggleTrainSelection(socketCl, menuSelection, executeAction);
			  break;
			  case SPEED_TR_SELECTOR:
			  	currentScreen = modifyTrainSpeedSelection(menuSelection, executeAction);
			  break;
			  case SPEED_PICKER:
			  	currentScreen = pickSpeed(socketCl, menuSelection, potenciometer, executeAction);
			  break;
			}
		}
	    
		std::this_thread::sleep_for(std::chrono::milliseconds(33));
		system("clear");
	}

	return 0;
}