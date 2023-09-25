#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

// global var tha is the default settings of our terminal
struct termios orig_termios;

void terminate(const char *s){
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
    // atexit runs this code when the program exits
    atexit(disableRawMode);

    struct termios raw = orig_termios;

    // dont echo what the user types into the shell by modifying the flags
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    // now set the terminals attributes to reflect raw mode
    // TCSAFLUSH waits for pending output to be printed to screen
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1){
        terminate("tcsetattr");
    }
}

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
                printf("%d\r\n", c);
            } else{
                printf("%d( %c )\r\n", c, c);
            }
        if (c == 'q'){
            break;
        }  
    }
    return 0;
}
