#include <stdlib.h>
#include <string.h>
#include "../include/keymap.h"
#include "../include/editor.h"

typedef union {
    const char* str;
    enum editorKey code;
} KeyValue;

typedef struct {
    KeyValue key;
    EditorAction action;
    const char* desc;
    int is_special_key;
} Keymap;

#define STR_KEYMAP_ENTRY(k, a, d) { \
    .key = { .str = k }, \
    .action = (a), \
    .desc = (d), \
    .is_special_key = 0 \
}

// Macro to simplify keymap entry creation for special keys
#define SPECIAL_KEYMAP_ENTRY(k, a, d) { \
    .key = { .code = (k) }, \
    .action = (a), \
    .desc = (d), \
    .is_special_key = 1 \
}

int normal_mode_keymap_length = 6;
Keymap normal_mode_keymap[] = {
    STR_KEYMAP_ENTRY("i", ACTION_ENTER_INSERT_MODE, "Enter insert mode"),
    STR_KEYMAP_ENTRY("v", ACTION_ENTER_VISUAL_MODE, "Enter visual mode"),
    SPECIAL_KEYMAP_ENTRY(ARROW_RIGHT, ACTION_MOVE_CURSOR_RIGHT, "Move cursor right"),
    SPECIAL_KEYMAP_ENTRY(ARROW_LEFT, ACTION_MOVE_CURSOR_LEFT, "Move cursor left"),
    SPECIAL_KEYMAP_ENTRY(ARROW_UP, ACTION_MOVE_CURSOR_UP, "Move cursor up"),
    SPECIAL_KEYMAP_ENTRY(ARROW_DOWN, ACTION_MOVE_CURSOR_DOWN, "Move cursor down"),
};

int insert_mode_keymap_length = 1;
Keymap insert_mode_keymap[] = {
    STR_KEYMAP_ENTRY("<ESC>", ACTION_ENTER_NORMAL_MODE, "Exit insert mode"),
};

int visual_mode_keymap_length = 1;
Keymap visual_mode_keymap[] = {
    STR_KEYMAP_ENTRY("<ESC>", ACTION_ENTER_NORMAL_MODE, "Exit visual mode"),
};

Keymap* currKeymap = normal_mode_keymap;
int* currKeymap_length = &normal_mode_keymap_length;

EditorAction keyMapLookup(const char* key, enum editorKey code) {
    for (int i = 0; i < *currKeymap_length; i++) {
        if (currKeymap[i].is_special_key) {
            if (currKeymap[i].key.code == code) return  currKeymap[i].action;
        } else {
            if (strcmp(key, currKeymap[i].key.str) == 0) return  currKeymap[i].action;
        }
    }
    return ACTION_UNKOWN;
}


int getEditorActionFromKey(int mode, const char* key, int code) {
    switch (mode) {
        case EDITOR_NORMAL_MODE:
            currKeymap = normal_mode_keymap;
            currKeymap_length= &normal_mode_keymap_length;
            break;
        case EDITOR_INSERT_MODE:
            currKeymap = insert_mode_keymap;
            currKeymap_length = &insert_mode_keymap_length;
            break;
        case EDITOR_VISUAL_MODE:
            currKeymap = visual_mode_keymap;
            currKeymap_length = &visual_mode_keymap_length;
            break;
        case EDITOR_COMMAND_MODE:
            currKeymap = NULL;
            *currKeymap_length = 0;
            break;
    }
    return keyMapLookup(key, code);
}
