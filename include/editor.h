#include <termios.h>
#include <time.h>

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

typedef enum {
    ACTION_UNKOWN,
    ACTION_ENTER_INSERT_MODE,
    ACTION_ENTER_VISUAL_MODE,
    ACTION_ENTER_NORMAL_MODE,
    ACTION_MOVE_CURSOR_RIGHT,
    ACTION_MOVE_CURSOR_LEFT,
    ACTION_MOVE_CURSOR_UP,
    ACTION_MOVE_CURSOR_DOWN,
    ACTION_FS_SAVE_FILE,
    ACTION_FS_OPEN_FILE,
    ACTION_EDITOR_WINDOWS_CYCLE_FORWARD,
    ACTION_EDITOR_WINDOWS_CYCLE_BACKWARD,
    ACTION_EDITOR_WINDOWS_EXIT,
    ACTION_EXIT_EDITOR,
} EditorAction;

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

struct EditorConfig{
    // cursor position
    int cx, cy;
    int rx;
    EditorMode mode;
    // screen offsets for moving cursor off screen
    int rowOffset;
    int columnOffset;
    int dirty;
    char* infoLine;
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
    struct EditorConfig editors[SCREEN_MAX+1];
    int screenNumber;
    // default terminal settings
    struct termios orig_termios;
};
