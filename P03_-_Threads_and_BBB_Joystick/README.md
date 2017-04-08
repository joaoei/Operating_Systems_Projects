# Project 03
All assignments were done in the C++ language for the Operational Systems subject.
UFRN - 2017.

## Contributors
Gustavo Araújo Carvalho

Thiago César de Lucena

### Assignment 3:
This project is intended to implement a joystick-like input for a terminal-based game of our choice using BeagleBone Black and electronic components. To do so, we forked an open source top-down shooter available on Github and modified the original project to fit this project's needings.

The electronic components used for specific functionalities on the joystick were a potentiometer, a LDR (Light Dependent Resistor) and a push button. To develop the joystick coding, it was necessary to use threads to manage the priority of each component's funcionality. Each component has its own thread, and their priority is listed as follows:

1. Potentiometer, controlling player's horizontal position.
2. LDR, controlling shooting action.
3. Push button, used for bomb action.

When a higher priority component is being used, the lower priority ones have their actions blocked. E.g., when the potentiometer is being used to move the spaceship, the player cannot 
use the LDR to shoot or the push button to drop a bomb.

For more information about the game, BeagleBone configuration, and access to the source codes, check the forked repository: https://github.com/GustavoAC/Lander