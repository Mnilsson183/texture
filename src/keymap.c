#include <stdlib.h>
#include <string.h>
#include "../include/keymap.h"
#include "../include/editor.h"

typedef struct {
    int key;
    EditorAction action;
    const char* desc;
} Map;

typedef struct {
    int size;
    Map maps[];
} Keymap;

void KeymapAdd(Keymap* keymap, Map map) {
    keymap->size++;

}

int normal_mode_keymap_length = 6;
Keymap normal_mode_keymap = {
    6,
    {
        {'i', ACTION_ENTER_INSERT_MODE, "Enter insert mode"},
        {'v', ACTION_ENTER_VISUAL_MODE, "Enter visual mode"},
        {ARROW_RIGHT, ACTION_MOVE_CURSOR_RIGHT, "Move cursor right"},
        {ARROW_LEFT, ACTION_MOVE_CURSOR_LEFT, "Move cursor left"},
        {ARROW_UP, ACTION_MOVE_CURSOR_UP, "Move cursor up"},
        {ARROW_DOWN, ACTION_MOVE_CURSOR_DOWN, "Move cursor down"},
    }
};

int insert_mode_keymap_length = 1;
Keymap insert_mode_keymap[] = {
    {'\x1b', ACTION_ENTER_NORMAL_MODE, "Exit insert mode"},
};

int visual_mode_keymap_length = 1;
Keymap visual_mode_keymap[] = {
    {'\x1b', ACTION_ENTER_NORMAL_MODE, "Exit visual mode"},
};

Keymap* currKeymap = &normal_mode_keymap;
int* currKeymap_length = &normal_mode_keymap_length;

EditorAction keyMapLookup(int key) {
    for (int i = 0; i < *currKeymap_length; i++) {
        if (key == currKeymap[i]->key) return currKeymap[i].action;
    }
    return ACTION_UNKOWN;
}


int getEditorActionFromKey(EditorMode mode, int key) {
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
    return keyMapLookup(key);
}
