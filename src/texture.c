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
#include "../include/logger.h"
#include "../include/highlight.h"

#define lambda(return_type, function_body) \
({ \
      return_type __fn__ function_body \
          __fn__; \
})
// int (*max)(int, int) = lambda (int, (int x, int y) { return x > y ? x : y; });
// max(4, 5); // Example

/** DEFINES**/
#define true 1
#define false 0

#define APPEND_INIT {NULL, 0}
/* prototypes */
void editorSetStatusMessage(const char *fmt, ...);
void editorRefreshScreen(void);
char *editorPrompt(char *prompt, void (*callback)(char *, int));

// global var that is the default settings of terminal

struct Editor E;

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

/* Editor Functions */
void editorDeleteChar(void){
    if(E.editors[E.screenNumber].cy == E.editors[E.screenNumber].displayLength){
        return;
    }
    if(E.editors[E.screenNumber].cx == 0 && E.editors[E.screenNumber].cy == 0){
        return;
    }

    EditorRow *row = &E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy];

    if(E.editors[E.screenNumber].cx > 0){
        editorRowDeleteChar(row, E.editors[E.screenNumber].cx - 1, &E.editors[E.screenNumber].dirty);
        E.editors[E.screenNumber].cx--;
    } else{
        E.editors[E.screenNumber].cx = E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy - 1].size;
        editorRowAppendString(&E.editors[E.screenNumber].row[E.editors[E.screenNumber].cy - 1], row->chars, row->size, &E.editors[E.screenNumber].dirty);
        editorDeleteRow(E.editors[E.screenNumber].cy, &E.editors[E.screenNumber]);
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
    initBuffer(&E, E.screenNumber);

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
char* editorPrompt(char *prompt, void (*callback)(char *, int)){
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
    char* s;
    E.logger->add(E.logger, "%s", input);
    switch (action) {
        case ACTION_UNKOWN: return;
        case ACTION_IGNORE: return;
        case ACTION_GET_INPUT:
            s = editorPrompt(":", NULL);
            getEditorActionFromKey(EDITOR_COMMAND_MODE, s);
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
            initBuffer(&E, E.screenNumber);
            break;
        case ACTION_EXIT_EDITOR:
            editorQuitTexture();
            break;
        case ACTION_INSERT_NEWLINE:
            editorInsertNewLine(&E.editors[E.screenNumber]);
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

        // not done yet
        case ACTION_REMOVE_LINE:
            break;
    }
}

void editorProcessKeyPress(void) {
    int c = editorReadKey();
    editorAppendActionBuffer(c);

    const char* actionBuf = E.editors[E.screenNumber].actionBuffer;
    editorSetStatusMessage("%s", actionBuf);

    EditorMode mode = E.editors[E.screenNumber].mode;

    EditorAction action = getEditorActionFromKey(mode, actionBuf);

    if (mode == EDITOR_INSERT_MODE && action == ACTION_UNKOWN) {
        editorInsertChar(c, &E.editors[E.screenNumber]);
        //fprintf(E.logger->file, "Unknown action %d", c);
    } else if(action != ACTION_UNKOWN){
        editorPreformEditorAction(action, actionBuf);
    }
    E.editors[E.screenNumber].actionBuffer[0] = '\0';
    editorSetStatusMessage("%s", actionBuf);
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
                initBuffer(&E, E.screenNumber);
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
                editorInsertNewLine(&E.editors[E.screenNumber]);
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
            editorInsertChar(c, &E.editors[E.screenNumber]);
            break;
        }
    } else if(E.editors[E.screenNumber].mode == EDITOR_VISUAL_MODE){
        switch (c){
        case CTRL_KEY('l'):
            case '\x1b':
            E.editors[E.screenNumber].mode = EDITOR_NORMAL_MODE;
                break;
            default:
            editorInsertChar(c, &E.editors[E.screenNumber]);
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

void editorRefreshScreen(void){
    editorScroll();

    struct AppendBuffer ab = APPEND_INIT;

    // hide the cursor
    abAppend(&ab, "\x1b[?25l", 6);
    // move the cursor to the 1,1 position in the terminal
    abAppend(&ab, "\x1b[H", 3);

    editorDrawRows(&E, &ab);
    editorDrawStatusBar(&E, &ab);
    editorDrawMessageBar(&E, &ab);

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
void initPlugins(void) {
    initKeymaps();
}

int main(int argc, char* argv[]){
    enableRawMode(E);
    initEditor(&E);
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
