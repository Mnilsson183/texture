// INCLUDES
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
// INCLUDES


// TERMINAL CONTROLS

// global var tha is the default settings of terminal
struct termios orig_termios;

void terminate(const char *s){
    // function to deal with the error outputs
    perror(s);
    exit(1);
}

void disableRawMode(void)
{
    // set the terminal attributes to the original values
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1){
        terminate("tcsetattr");
    }
}
void enableRawMode(void)
{
    // function to enter raw mode of the terminal

    // tcgetattr reads the terminal attributes
    if(tcgetattr(STDIN_FILENO, &orig_termios) == -1){
        terminate("tcgetattr");
    }
    // atexit disable the raw mode once the program finishes running
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    // terminal flags and other specifiers that allow out program to output
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    
    // set the parameters of the read function
    // VMIN is the number of characters read before stopping reading
    raw.c_cc[VMIN] = 0;
    // VTIME sets the timeout of the terminal read function
    raw.c_cc[VTIME] = 1;

    // Set the terminals attributes to reflect raw mode
    // TCSAFLUSH waits for pending output to be printed to screen
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
        terminate("tcsetattr");
    }
}
// TERMINAL CONTROLS

int main(void)
{
    enableRawMode();
    
    while (true)
    {
        char c = '\0';
        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN){
            terminate("read");
        }

        if(iscntrl(c)){
            // print the ascii of any control characters followed by a carrige return and a new line characters
            printf("%d\r\n", c);
        } else{
            // print both the ascii and the value followed by a carrige return and a new line characters
            printf("%d( %c )\r\n", c, c);
        }

        // q is the exit condition of the program
        if (c == 'q'){
            break;
        }  
    }
    return 0;
}
