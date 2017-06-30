#include <termios.h>
// #include <stdio.h>
#include <unistd.h>

class BufferToggle
{
    private:
        struct termios t;

    public:

        /*
         * Disables buffered input
         */

        void off(void)
        {
            tcgetattr(STDIN_FILENO, &t); //get the current terminal I/O structure
            t.c_lflag &= ~ICANON; //Manipulate the flag bits to do what you want it to do
            tcsetattr(STDIN_FILENO, TCSANOW, &t); //Apply the new settings
        }


        /*
         * Enables buffered input
         */

        void on(void)
        {
            tcgetattr(STDIN_FILENO, &t); //get the current terminal I/O structure
            t.c_lflag |= ICANON; //Manipulate the flag bits to do what you want it to do
            tcsetattr(STDIN_FILENO, TCSANOW, &t); //Apply the new settings
        }
};

// int main() {
//     BufferToggle bt;
//     bt.off();
//     char c;
//     while (true) {
//         c = getchar();
//         printf("%c", c+3);
//     }
// }