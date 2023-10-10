// basic c text editor

/** INCLUDES **/
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_source

#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

/** DEFINES**/
#define CTRL_KEY(key) ((key) & 0x1f)

/* editor options */
#define TEXTURE_VERSION "0.01"
#define TEXTURE_TAB_STOP 8

enum editorKey{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN,
    DEL_KEY,
    HOME_KEY,
    END_KEY,
    PAGE_UP,
    PAGE_DOWN,
};
/** DATA **/
typedef struct EditorRow{
    int size;
    int renderSize;
    char* chars;
    char* render;
} EditorRow;


// global var that is the default settings of terminal
struct EditorConfig{
    // cursor position
    int cx, cy;
    int rx;
    // screen offsets for moving cursor off screen
    int rowOffset;
    int columnOffset;
    // default terminal settings
    struct termios orig_termios;
    // rows and columns of the terminal
    int screenRows, screenColumns;
    int displayLength;
    EditorRow* row;
    char* fileName;
    char statusMessage[80];
    time_t statusMessage_time;
};
struct EditorConfig E;


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
    // first char is the first char of ASCII escape code
    if (c == '\x1b'){
        // sequence of 3 chars first being the ASCII escape code
        char seq[3];

        // read the STDIN and set both the first and second characters in seq
        if (read(STDIN_FILENO, &seq[0], 1) != 1){
            return '\x1b';
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1){
            return '\x1b';
        }
        
        // first char being the special char
        if(seq[0] == '['){
            // check that that seq falls into the bounds of our answer
            if (seq[1] >= '0' && seq[1] <= '9'){
                // read the STDIN to seq
                if (read(STDIN_FILENO, &seq[2], 1) != 1){
                    return '\x1b';
                }
                // check if last char is ~ denoting a special charater seq
                if (seq[2] == '~'){
                    // check the numeric value in the form of a char is equal to the following ASCII codes
                    switch (seq[1]){
                        case '1': return HOME_KEY;
                        case '3': return DEL_KEY;
                        case '4': return END_KEY;
                        case '5': return PAGE_UP;
                        case '6': return PAGE_DOWN;
                        case '7': return HOME_KEY;
                        case '8': return END_KEY;     
                    }
                }
            // another check if the keyboard gives value of a alpha form
            } else{
                switch (seq[1]){
                    case 'A': return ARROW_UP;
                    case 'B': return ARROW_DOWN;
                    case 'C': return ARROW_RIGHT;
                    case 'D': return ARROW_LEFT;
                    case 'H': return HOME_KEY;                      
                    case 'F': return END_KEY;
                }
            }
        // if the value of the first char is a 0 also denoting a special charater
        } else if(seq[0] == '0'){
            switch (seq[1]){
                case 'H': return HOME_KEY;
                case 'F': return END_KEY;
            }
        }
        return '\x1b';
    } else {
        // if no issue each key press
        return c;
    }
}

int getCursorPosition(int* rows, int* columns){
    // man read to get the cursor position
    char buf[32];
    unsigned int i = 0;
    // write a cursor position report
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4){
        return -1;
    }

    // 
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

    // easy way to do the getCursorPosition function
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0){
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12){
            return getCursorPosition(rows, columns);
        }
        // if the easy way fails use the man one
        editorReadKey();
        return -1;
    } else{
        *columns = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
/* row operations */

int editorRowCxtoRx(EditorRow *row, int cx){
    int rx = 0;
    int j;
    for(j = 0; j < cx; j++){
        if (row->chars[j] == 't'){
            rx += (TEXTURE_TAB_STOP - 1) - (rx % TEXTURE_TAB_STOP);
        }
        rx++;
    }
    return rx;
}

void editorUpdateRow(EditorRow *row){
    int tabs = 0;
    int j;
    for (j = 0; j < row->size; j++){
        if (row->chars[j] == '\t'){
            tabs++;
        }
    }
    free(row->render);
    row->render = malloc(row->size + ( tabs * (TEXTURE_TAB_STOP - 1)) + 1);

    int tempLength = 0;
    for (j = 0; j < row->size; j++){
        if (row->chars[j] == '\t'){
            row->render[tempLength++] = ' ';
            while (tempLength % TEXTURE_TAB_STOP != 0){
                row->render[tempLength++] = ' ';
            }
        } else{
            row->render[tempLength++] = row->chars[j];
        }
    }
    row->render[tempLength] = '\0';
    row->renderSize = tempLength;
}

void editorAppendRow(char* s, size_t length){
    E.row = realloc(E.row, sizeof(EditorRow) * (E.displayLength + 1));

    // add a row to display
    int at = E.displayLength;
    E.row[at].size = length;
    E.row[at].chars = malloc(length + 1);
    memcpy(E.row[at].chars, s, length);
    E.row[at].chars[length] = '\0';

    E.row[at].renderSize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at]);

    E.displayLength++;
}

void editorRowInsertChar(EditorRow *row, int at, int c){
    if (at < 0 || at > row->size){ 
        at = row->size;
    }
    row->chars = realloc(row->chars, row->size + 2);
        memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
        row->size++;
        row->chars[at] = c;
        editorUpdateRow(row);
}

/* Editor Functions */
void editorInsertChar(int c){
    if (E.cy == E.displayLength){
        editorAppendRow("", 0);
    }
    editorRowInsertChar(&E.row[E.cy], E.cx, c);
    E.cx++;
}


/* file i/o */
void editorOpen(char* filename){
    // open a file given a file path
    free(E.fileName);
    E.fileName = strdup(filename);

    FILE *filePath = fopen(filename, "r");
    if (!filePath){
        terminate("fopen");
    }
    char *line = NULL;
    size_t lineCap = 0;
    ssize_t lineLength;
    // read each line from this file into the row editorRow data struct chars feild
    while((lineLength = getline(&line, &lineCap, filePath)) != -1){
        // no need to read the carrige return and new line character
        while ((lineLength > 0) && ((line[lineLength - 1] == '\r') || 
                                    (line[lineLength - 1] == '\n')))
        {
            lineLength--;
            editorAppendRow(line, lineLength);
        }
    }
    free(line);
    fclose(filePath);
}

/* APPEND BUFFER */
struct AppendBuffer{
    // buffer to minimize write to terminal functions
    char *b;
    int len;
};

#define APPEND_INIT {NULL, 0}

void abAppend(struct AppendBuffer *ab, const char *s, int len){
    // append  to the appendBuffer 
    // give more memory to the information field of the struct
    char* new = realloc(ab->b, ab->len + len);

    // error check
    if (new == NULL){
        return;
    }
    // copy the bytes of s to the end of the new data structure
    memcpy(&new[ab->len], s, len);
    // assign to the old appendBuffer struct the new values with the included information
    ab->b = new;
    ab->len += len;
}

void abFree(struct AppendBuffer *ab){
    // free the data struct
    free(ab->b);
}

/** INPUT**/
void editorMoveCursor(int key){
    EditorRow* row = (E.cy >= E.displayLength) ? NULL: &E.row[E.cy];
    
    // update the cursor position based on the key inputs
    switch (key)
    {
        case ARROW_LEFT:
            if (E.cx != 0){
                E.cx--;
            } else if(E.cy > 0){
                E.cy--;
                E.cx = E.row[E.cy].size;
            }
            break;
        case ARROW_RIGHT:
            if (row && E.cx < row->size){
                E.cx++;
            // if go right on a the end of line
            } else if(row && E.cx == row->size){
                E.cy++;
                E.cx = 0;
            }
            break;
        case ARROW_UP:
            if (E.cy != 0){
                E.cy--;
            }
            break;
        case ARROW_DOWN:
            if (E.cy < E.displayLength){
                E.cy++;
            }
            break;
    }

    // snap cursor to the end of line
    row = (E.cy >= E.displayLength) ? NULL : &E.row[E.cy];
    int rowLength = row ? row->size : 0;
    if (E.cx > rowLength){
        E.cx = rowLength;
    }
}

void editorProcessKeyPress(void){
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

        // home key sets the x position to the home 
        case HOME_KEY:
            E.cx = 0;
            break;
        // end key sets the x position to the column before the end of the screen
        case END_KEY:
            if (E.cy < E.displayLength){
                E.cx = E.row[E.cy].size;
            }
            break;

        // send the cursor to the top of the column in cases up and down
        case PAGE_UP:
        case PAGE_DOWN:
            {
                if (c == PAGE_UP){
                    E.cy = E.rowOffset;
                } else if(c == PAGE_DOWN){
                    E.cy = E.rowOffset + E.screenRows - 1;
                }

                if (E.cy > E.displayLength){
                    E.cy = E.displayLength;
                }

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
void editorScroll(){
    // moving the screen around the file
    if (E.cy < E.displayLength){
        E.rx = editorRowCxtoRx(&E.row[E.cy], E.cx);
    }

    if (E.cy < E.rowOffset){
        E.rowOffset = E.cy;
    }
    if (E.cy >= E.rowOffset + E.screenRows){
        E.rowOffset = E.cy - E.screenRows + 1;
    }
    if (E.rx < E.columnOffset){
        E.columnOffset = E.rx;
    }
    if (E.rx >= E.columnOffset + E.screenColumns){
        E.columnOffset = E.rx - E.screenColumns + 1;
    }
}


void editorDrawRows(struct AppendBuffer *ab){
    // draw stuff
    int row;
    for(row = 0; row < E.screenRows; row++){
        int fileRow = row + E.rowOffset;
        if (fileRow >= E.displayLength){
                // put welcome message 1/3 down the screen
                if ((E.displayLength == 0) && (row == E.screenRows / 3)){
                    char welcome[80];
                    int welcomeLength = snprintf(welcome, sizeof(welcome),
                    "Texture Editor -- Version %s", TEXTURE_VERSION);
                    // if screen size is too small to fit the welcome message cut it off
                    if (welcomeLength > E.screenColumns){
                        welcomeLength = E.screenColumns;
                    }
                    // put the message in the middle of the screen
                    int padding = (E.screenColumns - welcomeLength) / 2;
                    if (padding){
                        abAppend(ab, "~", 1);
                        padding--;
                    }
                    while (padding--){
                        abAppend(ab, " ",  1);
                    }
                    abAppend(ab, welcome, welcomeLength);
                } else{
                    abAppend(ab, "~", 1);
                }
            } else {
                // else write the val in the column
                int length = E.row[fileRow].renderSize - E.columnOffset;
                if (length < 0){
                    length = 0;
                }
                if (length > E.screenColumns){
                    length = E.screenColumns;
                }
                abAppend(ab, &E.row[fileRow].render[E.columnOffset], length);
            }
            // erase from cursor to end of line
            abAppend(ab, "\x1b[K", 3);
            // print to the next line
            abAppend(ab, "\r\n", 2);
    }
}

void editorDrawStatusBar(struct AppendBuffer *ab){
    abAppend(ab, "\x1b[7m", 4);
    char status[80], rstatus[80];
    int length = snprintf(status, sizeof(status), "%.20s - %d lines", 
        E.fileName ? E.fileName : "[No Name]", E.displayLength);
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d%d", E.cy + 1, E.displayLength);
    if(length > E.screenColumns){
        length = E.screenColumns;
    }
    abAppend(ab , status, length);
    while(length < E.screenColumns){
        if (E.screenColumns - length == rlen){
            abAppend(ab, rstatus, rlen);
            break;
        } else{
            abAppend(ab, " ", 1);
            length++;
        }
    }
    abAppend(ab, "\x1b[m", 3);
    abAppend(ab, "\r\n", 2);
}

void editorDrawMessageBar(struct AppendBuffer *ab){
    abAppend(ab, "\x1b[K", 3);
    int messageLength = strlen(E.statusMessage);
    if (messageLength > E.screenColumns){
        messageLength = E.screenColumns;
    }
    if (messageLength && time(NULL) - E.statusMessage_time < 5){
        abAppend(ab, E.statusMessage, messageLength);
    }
}

void editorRefreshScreen(void){
    editorScroll();

    struct AppendBuffer ab = APPEND_INIT;

    // hide the cursor
    abAppend(&ab, "\x1b[?25l", 6);
    // move the cursor to the 1,1 position in the terminal
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&ab);
    editorDrawStatusBar(&ab);
    editorDrawMessageBar(&ab);

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH",   (E.cy - E.rowOffset) + 1, 
                                                (E.rx - E.columnOffset) + 1);
    abAppend(&ab, buf, strlen(buf));

    // show cursor again
    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorSetStatusMessage(const char *fmt, ...){
    va_list ap;
    va_start (ap, fmt);
    vsnprintf(E.statusMessage, sizeof(E.statusMessage), fmt, ap);
    va_end(ap);
    E.statusMessage_time = time(NULL);
}

/** INIT **/
void initEditor(void){
    // cursor positions
    E.cx = 0;
    E.cy = 0;
    E.rx = 0;
    E.rowOffset = 0;
    E.columnOffset = 0;
    E.displayLength = 0;
    E.row = NULL;
    E.fileName = NULL;
    E.statusMessage[0] = '\0';
    E.statusMessage_time = 0;

    if (getWindowSize(&E.screenRows, &E.screenColumns) == -1){
        terminate("getWindowSize");
    }
    E.screenRows = E.screenRows - 2;
}

int main(int argc, char* argv[]){
    enableRawMode();
    initEditor();
    // check the passed number of args
    if (argc >= 2){
        editorOpen(argv[1]);
    }

    editorSetStatusMessage("HELP: Ctrl-q to quit");
    
    while (true){
        editorRefreshScreen();
        editorProcessKeyPress();
    }
    return 0;
}
