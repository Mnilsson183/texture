#include <termios.h>
#include <time.h>

#ifndef EDITOR_H
#define EDITOR_H

#define TEXTURE_VERSION "2.01"
#define TEXTURE_TAB_STOP 4
#define TEXTURE_QUIT_TIMES 3
#define SCREEN_MAX 5
#define SCREEN_MIN 1

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

typedef enum {
    EDITOR_NORMAL_MODE,
    EDITOR_INSERT_MODE,
    EDITOR_VISUAL_MODE,
    EDITOR_COMMAND_MODE
} EditorMode;


enum editorKey{
    BACKSPACE = 127,
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

struct EditorSyntax {
    char* filetype;
    char** fileMatch;
    char** keywords;
    char* singleline_comment_start;
    char* multiline_comment_start;
    char* multiline_comment_end;
    int flags;
};

typedef struct EditorRow {
    int size;
    int renderSize;
    char* chars;
    char* render;
    unsigned char *highLight;
} EditorRow;

struct EditorBuffer{
    // cursor position
    int cx, cy;
    int rx;
    EditorMode mode;
    // screen offsets for moving cursor off screen
    int rowOffset;
    int columnOffset;
    int dirty;
    char* infoLine;
    char actionBuffer[40];
    // rows and columns of the terminal
    int screenRows, screenColumns;
    int displayLength;
    EditorRow* row;
    char* fileName;
    struct EditorSyntax *syntax;
    char statusMessage[80];
    time_t statusMessage_time;
};

struct EditorScreens{
    struct EditorBuffer editors[SCREEN_MAX+1];
    struct EditorBuffer* currBuffer;
    int screenNumber;
    // default terminal settings
    struct termios orig_termios;
};

int editorRowCxToRx(EditorRow *row, int cx);
int editorRowRxToCx(EditorRow *row, int rx);
void editorUpdateRow(EditorRow *row);
void editorFreeRow(EditorRow* row);
void editorInsertRow(int at, char* s, size_t length, struct EditorBuffer* buf);
void editorMoveCursor(int key, struct EditorBuffer* buf);

#endif
