
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

#include "../include/utils.h"
#include "../include/render.h"
#include "../include/highlight.h"
#include "../include/editor.h"

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

void editorDrawMessageBar(struct Editor* E, struct AppendBuffer *ab) {
    abAppend(ab, "\x1b[K", 3);
    int messageLength = strlen(E->editors[E->screenNumber].statusMessage);
    if (messageLength > E->editors[E->screenNumber].screenColumns) {
        messageLength = E->editors[E->screenNumber].screenColumns;
    }
    if (messageLength && time(NULL) - E->editors[E->screenNumber].statusMessage_time < 5) {
        abAppend(ab, E->editors[E->screenNumber].statusMessage, messageLength);
    }
}

void editorDrawRows(struct Editor* E, struct AppendBuffer *ab){
    // draw stuff
    int row;
    for(row = 0; row < E->editors[E->screenNumber].screenRows; row++){
        int fileRow = row + E->editors[E->screenNumber].rowOffset;
        if (fileRow >= E->editors[E->screenNumber].displayLength){
                // put welcome message 1/3 down the screen
                if ((E->editors[E->screenNumber].displayLength == 0) && (row == E->editors[E->screenNumber].screenRows / 3)){
                    char welcome[80];
                    int welcomeLength = snprintf(welcome, sizeof(welcome),
                    "Texture Editor -- Version %s", TEXTURE_VERSION);
                    // if screen size is too small to fit the welcome message cut it off
                    if (welcomeLength > E->editors[E->screenNumber].screenColumns){
                        welcomeLength = E->editors[E->screenNumber].screenColumns;
                    }
                    // put the message in the middle of the screen
                    int padding = (E->editors[E->screenNumber].screenColumns - welcomeLength) / 2;
                    if (padding){
                        abAppend(ab, "~", 1);
                        padding--;
                    }
                    while (padding--){
                        abAppend(ab, " ",  1);
                    }
                    abAppend(ab, welcome, welcomeLength);
                } else {
                    abAppend(ab, "~", 1);
                }
            } else {
                // else write the val in the column
                int length = E->editors[E->screenNumber].row[fileRow].renderSize - E->editors[E->screenNumber].columnOffset;
                if (length < 0){
                    length = 0;
                }
                if (length > E->editors[E->screenNumber].screenColumns){
                    length = E->editors[E->screenNumber].screenColumns;
                }
                char *c = &E->editors[E->screenNumber].row[fileRow].render[E->editors[E->screenNumber].columnOffset];
                unsigned char *highLight = &E->editors[E->screenNumber].row[fileRow].highLight[E->editors[E->screenNumber].columnOffset];
                int current_color = -1;
                int j;
                for(j = 0; j < length; j++){
                    if(iscntrl(c[j])){
                        char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                        abAppend(ab, "\x1b[7m", 4);
                        abAppend(ab, &sym, 1);
                        abAppend(ab, "\x1b[m", 3);
                        if(current_color != -1) {
                            char buf[16];
                            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
                            abAppend(ab, buf, clen);
                        }
                    } else if(highLight[j] == HL_NORMAL) {
                        if(current_color != -1){
                            abAppend(ab, "\x1b[39m", 5);
                            current_color = -1;
                        }
                        abAppend(ab, &c[j], 1);
                    } else{
                        int color = editorSyntaxToColor(highLight[j]);
                        if(color != current_color){
                            current_color = color;
                            char buffer[16];
                        int clen = snprintf(buffer, sizeof(buffer), "\x1b[%dm", color);
                        abAppend(ab, buffer, clen);
                        }
                        abAppend(ab, &c[j], 1);
                    }
                }
                abAppend(ab, "\x1b[39m", 5);
            }
            // erase from cursor to end of line
            abAppend(ab, "\x1b[K", 3);
            // print to the next line
            abAppend(ab, "\r\n", 2);
    }
}

void editorDrawStatusBar(struct Editor* E, struct AppendBuffer *ab){
    switch(E->editors[E->screenNumber].mode){
        default:
            abAppend(ab, "\x1b[7m", 4);
            break;
    }
    char status[80], rStatus[80];
    int length = snprintf(status, sizeof(status), "%.20s - %d lines %s - %s - screen number %d | %s",
        E->editors[E->screenNumber].fileName ? E->editors[E->screenNumber].fileName : "[No Name]", E->editors[E->screenNumber].displayLength,
        E->editors[E->screenNumber].dirty ? "(modified)": "",
        convertModeToString(E),
        E->screenNumber,
        E->editors[E->screenNumber].infoLine);
    int rlen = snprintf(rStatus, sizeof(rStatus),"%s | %d/%d",
        E->editors[E->screenNumber].syntax ? E->editors[E->screenNumber].syntax->filetype : "No Filetype",
        E->editors[E->screenNumber].cy + 1, 
        E->editors[E->screenNumber].displayLength);
    if(length > E->editors[E->screenNumber].screenColumns){
        length = E->editors[E->screenNumber].screenColumns;
    }
    abAppend(ab , status, length);
    while(length < E->editors[E->screenNumber].screenColumns){
        if (E->editors[E->screenNumber].screenColumns - length == rlen){
            abAppend(ab, rStatus, rlen);
            break;
        } else{
            abAppend(ab, " ", 1);
            length++;
        }
    }
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

void editorRefreshScreen(struct Editor* E){
    editorScroll(E);

    struct AppendBuffer ab = APPEND_INIT;

    // hide the cursor
    abAppend(&ab, "\x1b[?25l", 6);
    // move the cursor to the 1,1 position in the terminal
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(E, &ab);
    editorDrawStatusBar(E, &ab);
    editorDrawMessageBar(E, &ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH",   (E->editors[E->screenNumber].cy - E->editors[E->screenNumber].rowOffset) + 1, 
                                                (E->editors[E->screenNumber].rx - E->editors[E->screenNumber].columnOffset) + 1);
    abAppend(&ab, buf, strlen(buf));

    // show cursor again
    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}
