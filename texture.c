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

enum editorKey{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    PAGE_UP,
    PAGE_DOWN,
};
/** DATA **/

// global var tha is the default settings of terminal
struct editorConfig{
    // cursor spostions
    int cx, cy;
    // default terminal settings
    struct termios orig_termios;
    // rows and columns of the terminal
    int screenRows, screenColumns;
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
int editorReadKey() {
    int nread;
    char c;
    // read each key press
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN){
            terminate("read");
        }
    }
    if (c == '\x1b'){
        char seq[3];

        if (read(STDIN_FILENO, &seq[0], 1) != 1){
            return '\x1b';
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1){
            return '\x1b';
        }
        
        if(seq[0] == '['){
            if (seq[1] >= '0' && seq[1] <= '9'){
                if (read(STDIN_FILENO, &seq[2], 1) != 1){
                    return '\x1b';
                }
                if (seq[2] == '~'){
                    switch (seq[1])
                    {
                    case '5':
                        return PAGE_UP;
                    case '6':
                        return PAGE_DOWN;
                    }
                }
            } else{
                switch (seq[1]){
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                }
            }
        }
        return '\x1b';
    } else {
        // if no issue each key press
        return c;
    }
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
void editorMoveCursor(int key){
    switch (key)
    {
    case ARROW_LEFT:
        if (E.cx != 0){
            E.cx--;
        }
        break;
    case ARROW_RIGHT:
        if (E.cx != E.screenColumns - 1){
            E.cx++;
        }
        break;
    case ARROW_UP:
        if (E.cy != 0){
            E.cy--;
        }
        break;
    case ARROW_DOWN:
        if (E.cy != E.screenRows - 1){
            E.cy++;
        }
        break;
    }
}

void editorProcessKey(void){
    int c = editorReadKey();

    switch (c){
        // exit case
        case CTRL_KEY('q'):
            // write the 4 byte erase in display to the screen
            write(STDOUT_FILENO, "\x1b[2J", 4);
            // move the cursor to the 1,1 position in the terminal
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case PAGE_UP:
        case PAGE_DOWN:
            {

                int times = E.screenRows;
                while(times--){
                    editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN);
                }
            }
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c);
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

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH]", E.cy + 1, E.cx + 1);
    abAppend(&ab, buf, strlen(buf));

    abAppend(&ab, "\x1b[H", 3);
    // show cursor again
    abAppend(&ab, "\x1b[?25l", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}
/** INIT**/
void initEditor(void){
    // cursor positions
    E.cx = 0;
    E.cy = 0;

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
