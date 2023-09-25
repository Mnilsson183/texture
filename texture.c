#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// global var tha is the default settings of our terminal
struct termios orig_termios;

void disableRawMode(void)
{
    // set the terminal attributes to the original values
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
void enableRawMode(void)
{
    // function to enter raw mode of the terminal

    // tcgetattr reads the terminal attributes
    tcgetattr(STDIN_FILENO, &orig_termios);
    // atexit runs this code when the program exits
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    // dont echo what the user types into the shell by modifying the flags
    raw.c_lflag &= ~(ECHO | ICANON);

    // now set the terminals attributes to reflect raw mode
    // TCSAFLUSH waits for pending output to be printed to screen
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main(void)
{
    enableRawMode();
    

    char c;
    while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
        if(iscntrl(c)){
            printf("%d\n", c);
        } else{
            printf("%d( %c )\n", c, c);
        }
    }
    return 0;
}
