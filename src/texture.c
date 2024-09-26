// basic c text editor

/** INCLUDES **/
#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#define _GNU_source

#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#include "../include/dict.h"
#include "../include/utils.h"
#include "../include/render.h"
#include "../include/keymap.h"
#include "../include/editor.h"

/** DEFINES**/

#define true 1
#define false 0

#define APPEND_INIT {NULL, 0}

enum editorHighlight{
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MULTIPLE_LINE_COMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};
/* prototypes */
void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen(void);
char *editorPrompt(char *prompt, void (*callback)(char *, int));
void initBuffer(int screen);


// global var that is the default settings of terminal

struct EditorScreens E;

/* filetypes */
char* C_HL_extensions[] = {".c", ".h", ".cpp"};
char* C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", ""
};

char* Py_HL_extensions[] = {".py", ""};
char* Py_HL_keywords[] = {
    "if", "elif", "else", "def", "for"
};

struct EditorSyntax HighLightDataBase[] = {
    {"c",
    C_HL_extensions,
    C_HL_keywords,
    // temp change fix later # -> //
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
    
    {"py",
    Py_HL_extensions,
    Py_HL_keywords,
    "#", "", "",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};


#define HighLightDataBase_ENTRIES (sizeof(HighLightDataBase) / sizeof(HighLightDataBase[0]))

/* Syntax highlighting */
void editorUpdateSyntax(EditorRow *row){
    row->highLight = (unsigned char*)realloc(row->highLight, row->renderSize);
    memset(row->highLight, HL_NORMAL, row->renderSize);

    if(E.editors[E.screenNumber].syntax == NULL){
        return;
    }

    char** keywords = E.editors[E.screenNumber].syntax->keywords;

    const char *singleLightCommentStart = E.editors[E.screenNumber].syntax->singleline_comment_start;
    const char *multilineCommentStart = E.editors[E.screenNumber].syntax->multiline_comment_start;
    const char *multilineCommentEnd = E.editors[E.screenNumber].syntax->multiline_comment_end;

    int singleLightCommentStartLength = singleLightCommentStart ? strlen(singleLightCommentStart): 0;
    int multilineCommentStartLength = multilineCommentStart ? strlen(multilineCommentStart) : 0;
    int multilineCommentEndLength = multilineCommentEnd ? strlen(multilineCommentEnd) : 0;


    int prevSeparator = 1;
    int in_string = 0;
    int in_comment = 0;

    int i = 0;
    while (i < row->renderSize){
        char c = row->render[i];
        unsigned char prevHighlight = (i > 0) ? row->highLight[i - 1] : (char)HL_NORMAL;


        if(singleLightCommentStartLength && !in_string){
            if(!strncmp(&row->render[i], singleLightCommentStart, singleLightCommentStartLength)){
                memset(&row->highLight[i], HL_COMMENT, row->renderSize - i);
                break;
            }
        }

        if(multilineCommentStartLength && multilineCommentEndLength && !in_string){
            if(in_comment){
                row->highLight[i] = HL_MULTIPLE_LINE_COMMENT;
                if(!strncmp(&row->render[i], multilineCommentStart, multilineCommentStartLength)){
                    memset(&row->highLight[i], HL_MULTIPLE_LINE_COMMENT, multilineCommentStartLength);
                    i += 2;
                    in_comment = 0;
                    prevSeparator = 1;
                    continue;
                } else{
                    i++;
                    continue;
                }
            } else if(!strncmp(&row->render[i], multilineCommentStart, multilineCommentStartLength)){
                    memset(&row->highLight[i], HL_MULTIPLE_LINE_COMMENT, multilineCommentStartLength);
                    i += multilineCommentStartLength;
                    in_comment = 1;
                    continue;
            }
        }

        if(E.editors[E.screenNumber].syntax->flags & HL_HIGHLIGHT_STRINGS){
            if(in_string){
                if(c == '\\' && i + 1 < row->renderSize){
                    row->highLight[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                row->highLight[i] = HL_STRING;
                if(c == '\\' && i + 1 < row->renderSize){
                    row->highLight[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if(c == in_string){
                    in_string = 0;
                }
                i++;
                prevSeparator = 1;
                continue;
            } else{
                if(c == '"' || c == '\''){
                    in_string = c;
                    row->highLight[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if(E.editors[E.screenNumber].syntax->flags & HL_HIGHLIGHT_NUMBERS){
            if((isdigit(c) && (prevSeparator || prevHighlight == HL_NUMBER)) || 
            (c =='.' && prevHighlight == HL_NUMBER)){
                row->highLight[i] = HL_NUMBER;
                i++;
                prevSeparator = 0;
                continue;
            }
        }
        if(prevSeparator){
            int j;
            for(j = 0; !strcmp(keywords[j], ""); j++){
                int keywordLength = strlen(keywords[j]);
                int keyword2 = keywords[j][keywordLength - 1] == '|';
                if(keyword2) keywordLength--;

                if(!strncmp(&row->render[i], keywords[j], keywordLength) &&
                    isSeparator(row->render[i + keywordLength])){
                        memset(&row->highLight[i], keyword2 ? HL_KEYWORD2: HL_KEYWORD1, keywordLength);
                        i+=keywordLength;
                        break;
                }
            }
            if(!strcmp(keywords[j], "")){
                prevSeparator = isSeparator(c);
                i++;
            }
        }
        prevSeparator = isSeparator(c);
        i++;
    }
}

int editorSyntaxToColor(int highLight){
    switch (highLight)
    {
        case HL_COMMENT:
        case HL_MULTIPLE_LINE_COMMENT: return 36;
        case HL_KEYWORD1: return 33;
        case HL_KEYWORD2: return 32;
        case HL_NUMBER: return 31;
        case HL_STRING: return 35;
        case HL_MATCH: return 34;
        default: return 37;
    }
}

void editorSelectSyntaxHighlight(void){
    E.editors[E.screenNumber].syntax = NULL;
    if(E.editors[E.screenNumber].fileName == NULL){
        return;
    }

    char *extension = strrchr(E.editors[E.screenNumber].fileName, '.');
	
    for(unsigned int j = 0; j < HighLightDataBase_ENTRIES; j++){
        struct EditorSyntax *s = &HighLightDataBase[j];
        unsigned int i = 0;
        while(!strcmp(s->fileMatch[i], "")){
            int is_extension = (s->fileMatch[i][0] == '0');
            if((is_extension && extension && !strcmp(extension, s->fileMatch[i])) ||
                (!is_extension && strstr(E.editors[E.screenNumber].fileName, s->fileMatch[i]))){
                    E.editors[E.screenNumber].syntax = s;

                    int fileRow;
                    for(fileRow = 0; fileRow < E.editors[E.screenNumber].displayLength; fileRow++){
                        editorUpdateSyntax(&E.editors[E.screenNumber].row[fileRow]);
                    }

                    return;
                }
            i++;
        }
    }
}

/* row operations */

void editorDeleteRow(int at){
    if(at < 0 || at >= E.editors[E.screenNumber].displayLength){
        return;
    }
    editorFreeRow(&E.editors[E.screenNumber].row[at]);
    memmove(&E.editors[E.screenNumber].row[at], &E.editors[E.screenNumber].row[at + 1], sizeof(EditorRow) * (E.editors[E.screenNumber].displayLength - at - 1));
    E.editors[E.screenNumber].displayLength--;
    E.editors[E.screenNumber].dirty++;
}

void editorRowInsertChar(EditorRow *row, int at, int c){
    if (at < 0 || at > row->size){ 
        at = row->size;
    }
    row->chars = (char *)realloc(row->chars, row->size + 2);
        memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);
        row->size++;
        row->chars[at] = c;
        editorUpdateRow(row);
        E.editors[E.screenNumber].dirty++;
}

void editorRowAppendString(EditorRow *row, char *s, size_t length){
    row->chars = (char *)realloc(row->chars, row->size + length + 1);
    memcpy(&row->chars[row->size], s, length);
    row->size += length;
    row->chars[row->size] = '\0';
    editorUpdateRow(row);
    E.editors[E.screenNumber].dirty++;
}

void editorRowDeleteChar(EditorRow *row, int at){
    if (at < 0 || at >= row->size){
        return;
    }
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);
    row->size--;
    editorUpdateRow(row);
    E.editors[E.screenNumber].dirty++;
}

/* Editor Functions */
void editorInsertChar(int c){
    if (E.editors[E.screenNumber].cy == E.editors[E.screenNumber].displayLength){
        editorInsertRow(E.editors[E.screenNumber].displayLength, "", 0, &E.editors[E.screenNumber]);
    }
    editorRowInsertChar(&E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy], E.editors[E.screenNumber].cx, c);
    E.editors[E.screenNumber].cx++;
}

void editorInsertNewLine(void){
    if(E.editors[E.screenNumber].cx == 0){
        editorInsertRow(E.editors[E.screenNumber].cy, "", 0, &E.editors[E.screenNumber]);
    } else{
        EditorRow *row = &E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy];
        editorInsertRow(E.editors[E.screenNumber].cy + 1, &row->chars[E.editors[E.screenNumber].cx], row->size - E.editors[E.screenNumber].cx, &E.editors[E.screenNumber]);
        row = &E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy];
        row->size = E.editors[E.screenNumber].cx;
        row->chars[row->size] = '\0';
        editorUpdateRow(row);
    }
    E.editors[E.screenNumber].cy++;
    E.editors[E.screenNumber].cx = 0;
}


void editorDeleteChar(void){
    if(E.editors[E.screenNumber].cy == E.editors[E.screenNumber].displayLength){
        return;
    }
    if(E.editors[E.screenNumber].cx == 0 && E.editors[E.screenNumber].cy == 0){
        return;
    }

    EditorRow *row = &E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy];

    if(E.editors[E.screenNumber].cx > 0){
        editorRowDeleteChar(row, E.editors[E.screenNumber].cx - 1);
        E.editors[E.screenNumber].cx--;
    } else{
        E.editors[E.screenNumber].cx = E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy - 1].size;
        editorRowAppendString(&E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy - 1], row->chars, row->size);
        editorDeleteRow(E.editors[E.screenNumber].cy);
        E.editors[E.screenNumber].cy--;
    }
}


/* file i/o */

char* editorRowsToString(int* bufferlength){
    int totalLength = 0;
    int j;
    for(j = 0; j < E.editors[E.screenNumber].displayLength; j++){
        totalLength += E.editors[E.screenNumber].row[j].size + 1;
    }
    *bufferlength = totalLength;

    char *buf = (char *)malloc(totalLength);
    char *p = buf;
    for(j = 0; j < E.editors[E.screenNumber].displayLength; j++){
        memcpy(p, E.editors[E.screenNumber].row[j].chars, E.editors[E.screenNumber].row[j].size);
        p += E.editors[E.screenNumber].row[j].size;
        *p = '\n';
        p++;
    }
    return buf;
}

void editorOpen(const char* filename){
    // open a file given a file path
    if(E.editors[E.screenNumber].dirty){
        editorSetStatusMessage("WARNING!! file has unsaved changes. Please save changes of clear editor");
        return;
    }
    initBuffer(E.screenNumber);

    E.editors[E.screenNumber].fileName = (char* )filename;

    editorSelectSyntaxHighlight();

    FILE *filePath = fopen(filename, "r");
    if (!filePath){
        editorSetStatusMessage("file not found", NULL);
        E.editors[E.screenNumber].fileName = NULL;
        return;
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
            editorInsertRow(E.editors[E.screenNumber].displayLength, line, lineLength, &E.editors[E.screenNumber]);
        }
    }
    free(line);
    fclose(filePath);
    E.editors[E.screenNumber].dirty = 0;
}

void editorSave(void){
    if(E.editors[E.screenNumber].fileName == NULL){
        E.editors[E.screenNumber].fileName = editorPrompt("Save as (Esc to cancel): %s", NULL);
        if(E.editors[E.screenNumber].fileName == NULL){
            editorSetStatusMessage("Save aborted");
            return;
        }
        editorSelectSyntaxHighlight();
    }

    int length;
    char *buffer = editorRowsToString(&length);
    int fd = open(E.editors[E.screenNumber].fileName, O_RDWR | O_CREAT, 0644);
    if (fd != -1){
        if(ftruncate(fd, length) != -1){
            if(write(fd, buffer, length) == length){
                close(fd);
                free(buffer);
                E.editors[E.screenNumber].dirty = 0;
                editorSetStatusMessage("%d bytes written to disk", length);
                return;
            }
        }
        close(fd);
    }
    free(buffer);
    editorSetStatusMessage("Can't save! I/O error: %s", strerror(errno));
}

/* find */
void editorFindCallback(char *query, int key){
    static int last_match = -1;
    static int direction = 1;

    static int saved_highLight_line;
    static char *saved_highLight = NULL;

    if(saved_highLight){
        memcpy(E.editors[E.screenNumber].row[saved_highLight_line].highLight, saved_highLight, E.editors[E.screenNumber].row[saved_highLight_line].renderSize);
        free(saved_highLight);
        saved_highLight = NULL;
    }

    if(key == '\r' || key == '\x1b'){
        last_match = -1;
        direction = 1;
        return;
    } else if(key == ARROW_RIGHT || key == ARROW_DOWN){
        direction = 1;
    } else if(key == ARROW_LEFT || key == ARROW_UP){
        direction = -1;
    } else{
        last_match = -1;
        direction = 1;
    }

    if(last_match == -1){
        direction = 1;
    }
    int current = last_match;
    int i;
    for(i = 0; i < E.editors[E.screenNumber].displayLength; i++){
        current += direction;
        if(current == -1){
            current = E.editors[E.screenNumber].displayLength - 1;
        } else if(current == E.editors[E.screenNumber].displayLength){
            current = 0;
        }

        EditorRow *row = &E.editors[E.screenNumber].row[current];
        char *match = strstr(row->render, query);
        if(match){
            last_match = current;
            E.editors[E.screenNumber].cy = current;
            E.editors[E.screenNumber].cx = editorRowRxToCx(row, match - row->render);
            E.editors[E.screenNumber].rowOffset = E.editors[E.screenNumber].displayLength;

            saved_highLight_line = current;
            saved_highLight = (char *)malloc(row->size);
            memcpy(saved_highLight, row->highLight, row->renderSize);
            memset(&row->highLight[match - row->render], HL_MATCH, strlen(query));
            break;
        }
    }

}

void editorFind(void){
    int saved_cx = E.editors[E.screenNumber].cx;
    int saved_cy = E.editors[E.screenNumber].cy;
    int saved_columnOffset = E.editors[E.screenNumber].columnOffset;
    int saved_rowOffset = E.editors[E.screenNumber].rowOffset;

    char* query = editorPrompt("Search: %s (ESC/Arrows/Enter): ", editorFindCallback);
    if(query){
        free(query);
    } else {
        E.editors[E.screenNumber].cx = saved_cx;
        E.editors[E.screenNumber].cy = saved_cy;
        E.editors[E.screenNumber].columnOffset = saved_columnOffset;
        E.editors[E.screenNumber].rowOffset = saved_rowOffset;
    }
}

/** INPUT**/
char *editorPrompt(char *prompt, void (*callback)(char *, int)){
    size_t bufferSize = 128;
    char *buffer = (char *)malloc(bufferSize);

    size_t bufferLength = 0;
    buffer[0] = '\0';

    while (true){
        editorSetStatusMessage(prompt, buffer);
        editorRefreshScreen();

        int c = editorReadKey();
        if(c == DEL_KEY || c == CTRL_KEY('h') || c == BACKSPACE){
            if(bufferLength != 0){
                buffer[--bufferLength] = '\0';
            }
        } else if(c == '\x1b'){
            editorSetStatusMessage("");
            if(callback){
                callback(buffer, c);
            }
            free(buffer);
            return NULL;
        } else if(c == '\r'){
            if(bufferLength != 0){
                editorSetStatusMessage("");
                if(callback){
                    callback(buffer, c);
                }
                return buffer;
            }
        } else if(!iscntrl(c) && c < 128){
            if(bufferLength == bufferSize - 1){
                bufferSize *= 2;
                buffer = (char *)realloc(buffer, bufferSize);
                if(buffer == NULL){
                    free(buffer);
                    terminate("realloc error");
                }
            }
            buffer[bufferLength++] = c;
            buffer[bufferLength] = '\0';
        }

        if(callback){
            callback(buffer, c);
        }
    }
}

int editorSetRow(int row){
    if(row > E.editors[E.screenNumber].displayLength) row = E.editors[E.screenNumber].displayLength;
    else if(row < 0) row = 0;
    E.editors[E.screenNumber].cy = row;
    return 0;
}

void editorQuitTexture(void) {

    // write the 4 byte erase in display to the screen
    write(STDOUT_FILENO, "\x1b[2J", 4);
    // move the cursor to the 1,1 position in the terminal
    write(STDOUT_FILENO, "\x1b[H", 3);
    exit(0);
}

void handleCommand(const char* s){
    char command = s[0];
    printf("%c",command);
    // first char identifier
    size_t startIndex = 1;
    size_t i = startIndex;
    char* str = "";
    while(s[i] != '\0'){
        str = str + s[i];
        i++;
    }
    int lineNumber = atoi(str);
    switch(command){
        case 'q':
            editorQuitTexture();
        case 'l':
            if(editorSetRow(lineNumber) == -1){
                editorSetStatusMessage("line number is impossible");
                return;
            }
            break;
        // move + or - lines
        case '-':
            editorSetRow(E.editors[E.screenNumber].cy - lineNumber);
            break;
        case '+':
            editorSetRow(E.editors[E.screenNumber].cy + lineNumber);
            break;
    }
}

// cannot change E.screen number directly or might try to use non created value
void editorSwitchScreen(int diff){
    E.screenNumber += diff;
    if(E.screenNumber > SCREEN_MAX){
        E.screenNumber = SCREEN_MIN;
    }

    if(E.screenNumber < SCREEN_MIN){
        E.screenNumber = SCREEN_MAX;
    }
    E.currBuffer = &E.editors[E.screenNumber];
}

void editorAppendActionBuffer(char c) {
    int i = strlen(E.editors[E.screenNumber].actionBuffer);
    if (i == 40) return;
    E.editors[E.screenNumber].actionBuffer[i] = c;
    E.editors[E.screenNumber].actionBuffer[i + 1] = '\0';

}


void editorPreformEditorAction(EditorAction action, const char* input) {
    switch (action) {
        case ACTION_UNKOWN: return;
        case ACTION_IGNORE: return;
        case ACTION_EXECUTE_DIR:
            handleCommand(input);
            break;
        case ACTION_ENTER_INSERT_MODE:
            E.editors[E.screenNumber].mode = EDITOR_INSERT_MODE;
            break;
        case ACTION_ENTER_VISUAL_MODE:
            E.editors[E.screenNumber].mode = EDITOR_VISUAL_MODE;
            break;
        case ACTION_ENTER_NORMAL_MODE:
            E.editors[E.screenNumber].mode = EDITOR_NORMAL_MODE;
            break;
        case ACTION_MOVE_CURSOR_RIGHT:
        case ACTION_MOVE_CURSOR_LEFT:
        case ACTION_MOVE_CURSOR_UP:
        case ACTION_MOVE_CURSOR_DOWN:
            editorMoveCursor(action, &E.editors[E.screenNumber]);
            break;
        case ACTION_FS_SAVE_FILE:
            editorSave();
            break;
        case ACTION_FS_OPEN_FILE:
            editorOpen(input);
            break;
        case ACTION_EDITOR_WINDOWS_CYCLE_FORWARD:
            editorSwitchScreen(1);
            break;
        case ACTION_EDITOR_WINDOWS_CYCLE_BACKWARD:
            editorSwitchScreen(1);
            break;
        case ACTION_EDITOR_WINDOWS_EXIT:
            initBuffer(E.screenNumber);
            break;
        case ACTION_EXIT_EDITOR:
            editorQuitTexture();
            break;
        case ACTION_INSERT_NEWLINE:
            editorInsertNewLine();
            break;
        case ACTION_MOVE_HOME_KEY:
            E.editors[E.screenNumber].cx = 0;
            break;
        case ACTION_MOVE_END_KEY:
            if (E.editors[E.screenNumber].cy < E.editors[E.screenNumber].displayLength){
                E.editors[E.screenNumber].cx = E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy].size;
            }
            break;
        case ACTION_REMOVE_BACKSPACE:
            editorDeleteChar();
            break;
        case ACTION_REMOVE_DEL_KEY:
            editorMoveCursor(ARROW_RIGHT, &E.editors[E.screenNumber]);
            editorDeleteChar();
            break;
        // values are caught early
        case ACTION_WAIT:
        case ACTION_DISCARD:
            break;
    }
}

void editorProcessKeyPress(void) {
    int c = editorReadKey();
    editorAppendActionBuffer(c);
    editorSetStatusMessage("%s", E.editors[E.screenNumber].actionBuffer);
    EditorAction action = getEditorActionFromKey(E.editors[E.screenNumber].mode, E.editors[E.screenNumber].actionBuffer);
    if (E.editors[E.screenNumber].mode == EDITOR_INSERT_MODE && action == ACTION_UNKOWN) {
        editorInsertChar(c);
    } else {
        editorPreformEditorAction(action, NULL);
    }
    E.editors[E.screenNumber].actionBuffer[0] = '\0';
}

void editorProcessKeyPressBackup(void){
    int c = editorReadKey();
    static int quit_times = TEXTURE_QUIT_TIMES;


    if(E.editors[E.screenNumber].mode == EDITOR_NORMAL_MODE){
        switch(c){
            // switch mode
            case 'i':
                E.editors[E.screenNumber].mode = EDITOR_INSERT_MODE;
                break;
            case 'v':
                E.editors[E.screenNumber].mode = EDITOR_VISUAL_MODE;
                break;
            case ':':
                handleCommand(editorPrompt(": %s", NULL));
                break;
            case 'O':
                editorOpen(editorPrompt("Open file: %s", NULL));
                break;

            case CTRL_KEY('x'):
                editorSwitchScreen(1);
                break;
            case CTRL_KEY('z'):
                editorSwitchScreen(-1);
                break;
            // exit current
            case CTRL_KEY('c'):
                if(E.editors[E.screenNumber].dirty && quit_times > 0){
                    editorSetStatusMessage("WARNING!! file has unsaved changes. "
                    "Press Ctrl-C %d more times to quit", quit_times);
                    quit_times--;
                    return;
                }
                initBuffer(E.screenNumber);
                break;
            // exit all
            case CTRL_KEY('q'):
                if(E.editors[E.screenNumber].dirty && quit_times > 0){
                    editorSetStatusMessage("WARNING!! file has unsaved changes. "
                    "Press Ctrl-Q %d more times to quit", quit_times);
                    quit_times--;
                    return;
                }
                // write the 4 byte erase in display to the screen
                write(STDOUT_FILENO, "\x1b[2J", 4);
                // move the cursor to the 1,1 position in the terminal
                write(STDOUT_FILENO, "\x1b[H", 3);
                exit(0);
                break;
            case CTRL_KEY('s'):
                editorSave();
                break;
            case CTRL_KEY('f'):
                if(E.editors[E.screenNumber].cy < E.editors[E.screenNumber].displayLength){
                    editorFind();
                }
                break;
            case ARROW_UP:
            case ARROW_DOWN:
            case ARROW_LEFT:
            case ARROW_RIGHT:
                editorMoveCursor(c, &E.editors[E.screenNumber]);
                break;
            case CTRL_KEY('l'):
            case '\x1b':
                break;
        }
    } else if(E.editors[E.screenNumber].mode == EDITOR_INSERT_MODE){
        switch(c){

            case '\r':
                editorInsertNewLine();
                break;
            // home key sets the x position to the home 
            case HOME_KEY:
                E.editors[E.screenNumber].cx = 0;
                break;
            // end key sets the x position to the column before the end of the screen
            case END_KEY:
                if (E.editors[E.screenNumber].cy < E.editors[E.screenNumber].displayLength){
                    E.editors[E.screenNumber].cx = E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy].size;
                }
                break;
            case BACKSPACE:
            case CTRL_KEY('h'):
            case DEL_KEY:
                if(c == DEL_KEY){
                    editorMoveCursor(ARROW_RIGHT, &E.editors[E.screenNumber]);
                }
                editorDeleteChar();
                break;
            // send the cursor to the top of the column in cases up and down
            case PAGE_UP:
            case PAGE_DOWN:
                {
                    if (c == PAGE_UP){
                        E.editors[E.screenNumber].cy = E.editors[E.screenNumber].rowOffset;
                    } else if(c == PAGE_DOWN){
                        E.editors[E.screenNumber].cy = E.editors[E.screenNumber].rowOffset + E.editors[E.screenNumber].screenRows - 1;
                    }

                    if (E.editors[E.screenNumber].cy > E.editors[E.screenNumber].displayLength){
                        E.editors[E.screenNumber].cy = E.editors[E.screenNumber].displayLength;
                    }

                    int times = E.editors[E.screenNumber].screenRows;
                    while(times--){
                        editorMoveCursor(c == PAGE_UP ? ARROW_UP : ARROW_DOWN, &E.editors[E.screenNumber]);
                    }
                }
                break;
            case ARROW_UP:
            case ARROW_DOWN:
            case ARROW_LEFT:
            case ARROW_RIGHT:
                editorMoveCursor(c, &E.editors[E.screenNumber]);
                break;
            case CTRL_KEY('l'):
            case '\x1b':
            E.editors[E.screenNumber].mode = EDITOR_NORMAL_MODE;
                break;
            default:
            editorInsertChar(c);
            break;
        }
    } else if(E.editors[E.screenNumber].mode == EDITOR_VISUAL_MODE){
        switch (c){
        case CTRL_KEY('l'):
            case '\x1b':
            E.editors[E.screenNumber].mode = EDITOR_NORMAL_MODE;
                break;
            default:
            editorInsertChar(c);
            break;
        }
    } 
    quit_times = TEXTURE_QUIT_TIMES;
}

/** OUTPUT **/
void editorScroll(void){
    // moving the screen around the file
    if (E.editors[E.screenNumber].cy < E.editors[E.screenNumber].displayLength){
        E.editors[E.screenNumber].rx = editorRowCxToRx(&E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy], E.editors[E.screenNumber].cx);
    }

    if (E.editors[E.screenNumber].cy < E.editors[E.screenNumber].rowOffset){
        E.editors[E.screenNumber].rowOffset = E.editors[E.screenNumber].cy;
    }
    if (E.editors[E.screenNumber].cy >= E.editors[E.screenNumber].rowOffset + E.editors[E.screenNumber].screenRows){
        E.editors[E.screenNumber].rowOffset = E.editors[E.screenNumber].cy - E.editors[E.screenNumber].screenRows + 1;
    }
    if (E.editors[E.screenNumber].rx < E.editors[E.screenNumber].columnOffset){
        E.editors[E.screenNumber].columnOffset = E.editors[E.screenNumber].rx;
    }
    if (E.editors[E.screenNumber].rx >= E.editors[E.screenNumber].columnOffset + E.editors[E.screenNumber].screenColumns){
        E.editors[E.screenNumber].columnOffset = E.editors[E.screenNumber].rx - E.editors[E.screenNumber].screenColumns + 1;
    }
}

void editorDrawRows(struct AppendBuffer *ab){
    // draw stuff
    int row;
    for(row = 0; row < E.editors[E.screenNumber].screenRows; row++){
        int fileRow = row + E.editors[E.screenNumber].rowOffset;
        if (fileRow >= E.editors[E.screenNumber].displayLength){
                // put welcome message 1/3 down the screen
                if ((E.editors[E.screenNumber].displayLength == 0) && (row == E.editors[E.screenNumber].screenRows / 3)){
                    char welcome[80];
                    int welcomeLength = snprintf(welcome, sizeof(welcome),
                    "Texture Editor -- Version %s", TEXTURE_VERSION);
                    // if screen size is too small to fit the welcome message cut it off
                    if (welcomeLength > E.editors[E.screenNumber].screenColumns){
                        welcomeLength = E.editors[E.screenNumber].screenColumns;
                    }
                    // put the message in the middle of the screen
                    int padding = (E.editors[E.screenNumber].screenColumns - welcomeLength) / 2;
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
                int length = E.editors[E.screenNumber].row[fileRow].renderSize - E.editors[E.screenNumber].columnOffset;
                if (length < 0){
                    length = 0;
                }
                if (length > E.editors[E.screenNumber].screenColumns){
                    length = E.editors[E.screenNumber].screenColumns;
                }
                char *c = &E.editors[E.screenNumber].row[fileRow].render[E.editors[E.screenNumber].columnOffset];
                unsigned char *highLight = &E.editors[E.screenNumber].row[fileRow].highLight[E.editors[E.screenNumber].columnOffset];
                int current_color = -1;
                int j;
                for(j = 0; j < length; j++){
                    if(iscntrl(c[j])){
                        char sym = (c[j] <= 26) ? '@' + c[j] : '?';
                        abAppend(ab, "\x1b[7m", 4);
                        abAppend(ab, &sym, 1);
                        abAppend(ab, "\x1b[m", 3);
                        if(current_color != -1){
                            char buf[16];
                            int clen = snprintf(buf, sizeof(buf), "\x1b[%dm", current_color);
                            abAppend(ab, buf, clen);
                        }
                    } else if(highLight[j] == HL_NORMAL){
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

char* convertModeToString(void){
    switch (E.editors[E.screenNumber].mode){
        case EDITOR_NORMAL_MODE: return "normal";
        case EDITOR_INSERT_MODE: return "insert";
        case EDITOR_VISUAL_MODE: return "visual";
        case EDITOR_COMMAND_MODE: return "command";
        default: return "";
    }
}

void editorDrawStatusBar(struct AppendBuffer *ab){
    switch(E.editors[E.screenNumber].mode){
        default:
            abAppend(ab, "\x1b[7m", 4);
            break;
    }
    char status[80], rStatus[80];
    int length = snprintf(status, sizeof(status), "%.20s - %d lines %s - %s - screen number %d | %s",
        E.editors[E.screenNumber].fileName ? E.editors[E.screenNumber].fileName : "[No Name]", E.editors[E.screenNumber].displayLength,
        E.editors[E.screenNumber].dirty ? "(modified)": "",
        convertModeToString(),
        E.screenNumber,
        E.editors[E.screenNumber].infoLine);
    int rlen = snprintf(rStatus, sizeof(rStatus),"%s | %d/%d",
        E.editors[E.screenNumber].syntax ? E.editors[E.screenNumber].syntax->filetype : "No Filetype",
        E.editors[E.screenNumber].cy + 1, 
        E.editors[E.screenNumber].displayLength);
    if(length > E.editors[E.screenNumber].screenColumns){
        length = E.editors[E.screenNumber].screenColumns;
    }
    abAppend(ab , status, length);
    while(length < E.editors[E.screenNumber].screenColumns){
        if (E.editors[E.screenNumber].screenColumns - length == rlen){
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

void editorDrawMessageBar(struct AppendBuffer *ab){
    abAppend(ab, "\x1b[K", 3);
    int messageLength = strlen(E.editors[E.screenNumber].statusMessage);
    if (messageLength > E.editors[E.screenNumber].screenColumns){
        messageLength = E.editors[E.screenNumber].screenColumns;
    }
    if (messageLength && time(NULL) - E.editors[E.screenNumber].statusMessage_time < 5){
        abAppend(ab, E.editors[E.screenNumber].statusMessage, messageLength);
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
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH",   (E.editors[E.screenNumber].cy - E.editors[E.screenNumber].rowOffset) + 1, 
                                                (E.editors[E.screenNumber].rx - E.editors[E.screenNumber].columnOffset) + 1);
    abAppend(&ab, buf, strlen(buf));

    // show cursor again
    abAppend(&ab, "\x1b[?25h", 6);

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
}

void editorSetStatusMessage(const char *fmt, ...){
    va_list ap;
    va_start (ap, fmt);
    vsnprintf(E.editors[E.screenNumber].statusMessage, sizeof(E.editors[E.screenNumber].statusMessage), fmt, ap);
    va_end(ap);
    E.editors[E.screenNumber].statusMessage_time = time(NULL);
}

/** INIT **/
void initBuffer(int screen){
    // cursor positions
    E.editors[screen].cx = 0;
    E.editors[screen].cy = 0;
    E.editors[screen].rx = 0;
    E.editors[screen].mode = EDITOR_NORMAL_MODE;
    E.editors[screen].rowOffset = 0;
    E.editors[screen].columnOffset = 0;
    E.editors[screen].displayLength = 0;
    E.editors[screen].dirty = 0;
    E.editors[screen].row = NULL;
    E.editors[screen].fileName = NULL;
    E.editors[screen].statusMessage[0] = '\0';
    E.editors[screen].actionBuffer[0] = '\0';
    E.editors[screen].statusMessage_time = 0;
    E.editors[screen].syntax = NULL;

    if (getWindowSize(&E.editors[screen].screenRows, &E.editors[screen].screenColumns) == -1){
        terminate("getWindowSize");
    }
    E.editors[screen].screenRows = E.editors[screen].screenRows - 2;
}

void initEditor(void){
    for(int i = SCREEN_MIN; i <= SCREEN_MAX; i++){
        initBuffer(i);
    }
    E.screenNumber = SCREEN_MIN;
}

void initPlugins(void) {
    initKeymaps();
}

int main(int argc, char* argv[]){
    enableRawMode(E);
    initEditor();
    // check the passed number of args
    if (argc >= 2){
        editorOpen(argv[1]);
    }
    initPlugins();

    editorSetStatusMessage("HELP: Ctrl-q to quit | Ctrl-s to save | Ctrl-f find | 'O' open file");
    
    while (true){
        editorRefreshScreen();
        editorProcessKeyPress();
    }
    return 0;
}
