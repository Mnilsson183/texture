
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>

#include "../include/utils.h"
#include "../include/render.h"

struct termios* org_term;

void disableRawMode(){
    // set the terminal attributes to the original values
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, org_term) == -1){
        terminate("tcsetattr");
    }
}

void enableRawMode(struct Editor E){
    // function to enter raw mode of the terminal

    // tcgetattr reads the terminal attributes
    if(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1){
        terminate("tcgetattr");
    }
    org_term = &E.orig_termios;
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

void abAppend(struct AppendBuffer *ab, const char *s, int len){
    // append  to the appendBuffer 
    // give more memory to the information field of the struct
    char* newAppend = (char *)realloc(ab->b, ab->len + len);

    // error check
    if (newAppend == NULL){
        return;
    }
    // copy the bytes of s to the end of the new data structure
    memcpy(&newAppend[ab->len], s, len);
    // assign to the old appendBuffer struct the new values with the included information
    ab->b = newAppend;
    ab->len += len;
}

void abFree(struct AppendBuffer *ab){
    // free the data struct
    free(ab->b);
}
