// basic c text editor

/** INCLUDES **/
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/** DEFINES**/
#define CTRL_KEY(key) ((key) & 0x1f)


/** DATA VALUES**/

// global var tha is the default settings of terminal
struct editorConfig{
    struct termios orig_termios;
};
struct editorConfig E;

/** TERMINAL**/
void terminate(const char *s){
    // write the 4 byte erase in display to the screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // move the cursor to the 1,1 position in the terminal
    write(STDOUT_FILENO, "\x1b[H", 3);
    // function to deal with the error outputs
    perror(s);
    exit(1);
}

void disableRawMode(void)
{
    // set the terminal attributes to the original values
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1){
        terminate("tcsetattr");
    }
}
void enableRawMode(void)
{
    // function to enter raw mode of the terminal

    // tcgetattr reads the terminal attributes
    if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1){
        terminate("tcgetattr");
    }
    // atexit disable the raw mode once the program finishes running
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;

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

/** INPUT**/
char editorReadKey() {
    int nread;
    char c;
    // read each key press
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN){
            terminate("read");
        }
    }
    // if no issue return each key press
    return c;
}

void editorProcessKey(void){
    char c = editorReadKey();

    switch (c){
        // exit case
        case CTRL_KEY('q'):
            // write the 4 byte erase in display to the screen
            write(STDOUT_FILENO, "\x1b[2J", 4);
            // move the cursor to the 1,1 position in the terminal
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;
        }
}

/** OUTPUT **/
void editorDrawRows(void){
    // draw a ~ column
    int rows;
    for(rows = 0; rows < 24; rows++){
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen(void){
    // write the 4 byte erase in display to the screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // move the cursor to the 1,1 position in the terminal
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}
/** INIT**/
int main(void){
    enableRawMode();
    
    while (true){
        editorRefreshScreen();
        editorProcessKey();
    }
    return 0;
}
