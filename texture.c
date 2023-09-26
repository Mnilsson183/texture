// basic c text editor

/** INCLUDES **/
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/** DEFINES**/
#define CTRL_KEY(key) ((key) & 0x1f)
#define TEXTURE_VERSION "0.01"

/** DATA VALUES**/

// global var tha is the default settings of terminal
struct editorConfig{
    struct termios orig_termios;
    int screenRows;
    int screenColumns;
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

void disableRawMode(void){
    // set the terminal attributes to the original values
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1){
        terminate("tcsetattr");
    }
}
void enableRawMode(void){
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

int getCursorPosition(int* rows, int* columns){
    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4){
        return -1;
    }

    while (i < sizeof(buf) - 1){
        if (read(STDIN_FILENO, &buf[i], 1)){
            break;
        }
        if(buf[i] == 'R'){
            break;
        }
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != '\x1b' || buf[1] != '['){
        return -1;
    }
    if (sscanf(&buf[2],"%d;%d", rows, columns) != 2){
        return -1;
    }
    return 0;
    
}

int getWindowSize(int* rows, int* columns){
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12){
            return getCursorPosition(rows, columns);
        }
        editorReadKey();
        return -1;
    } else{
        *columns = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
/* APPEND BUFFER */
struct abuf{
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len){
    char* new = realloc(ab->b, ab->len + len);

    if (new == NULL){
        return;
    }
    memcpy(&new[ab->len], s, len);
    ab->b = new;
    ab->len += len;
}

void abFree(struct abuf *ab){
    free(ab->b);
}

/** INPUT**/

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
void editorDrawRows(struct abuf *ab){
    // draw a ~ column
    int rows;
    for(rows = 0; rows < E.screenRows; rows++){
        // put welcome message 1/3 down the screen
        if (rows == (3 *E.screenRows / 8)){
            char welcome[80];
            int welcomelen = snprintf(welcome, sizeof(welcome),
            "Texture editor -- version %s", TEXTURE_VERSION);
            if (welcomelen > E.screenColumns){
                welcomelen = E.screenColumns;
            }
            int padding = (E.screenColumns - welcomelen) / 2;
            if (padding){
                abAppend(ab, "~", 1);
                padding--;
            }
            while (padding--){
                abAppend(ab, " ",  1);
            }
            abAppend(ab, welcome, welcomelen);
        } else{
            abAppend(ab, "~", 1);
        }
        abAppend(ab, "\x1b[K", 3);
        if(rows < E.screenRows - 1){
            abAppend(ab, "\r\n", 2);
        }
    }
}

void editorRefreshScreen(void){
    struct abuf ab = ABUF_INIT;

    // hide the cursor
    abAppend(&ab, "\x1b[?25l", 6);
    // move the cursor to the 1,1 position in the terminal
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);

    abAppend(&ab, "\x1b[H", 3);
    // show cursor again
    abAppend(&ab, "\x1b[?25l", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}
/** INIT**/
void initEditor(void){
    if (getWindowSize(&E.screenRows, &E.screenColumns) == -1){
        terminate("getWindowSize");
    }
}

int main(void){
    enableRawMode();
    initEditor();
    
    while (true){
        editorRefreshScreen();
        editorProcessKey();
    }
    return 0;
}
