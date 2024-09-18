#include <stdlib.h>
#include <string.h>
#include "../include/keymap.h"
#include "../include/editor.h"
#include "../include/vector.h"

typedef struct {
    int key;
    EditorAction action;
    const char* desc;
} Map;

Map* moveMapToHeap(Map map) {
    Map* newMap = malloc(sizeof(Map));
    newMap->action = map.action;
    newMap->desc = map.desc;
    newMap->key = map.key;
    return newMap;
}

Map normal_keymap[] = {
    {'i', ACTION_ENTER_INSERT_MODE, "Enter insert mode"},
    {'v', ACTION_ENTER_VISUAL_MODE, "Enter visual mode"},
    {ARROW_RIGHT, ACTION_MOVE_CURSOR_RIGHT, "Move cursor right"},
    {ARROW_LEFT, ACTION_MOVE_CURSOR_LEFT, "Move cursor left"},
    {ARROW_UP, ACTION_MOVE_CURSOR_UP, "Move cursor up"},
    {ARROW_DOWN, ACTION_MOVE_CURSOR_DOWN, "Move cursor down"},
};
int normal_keymap_length = 6;

Map visual_keymap[] = {
    {'\x1b', ACTION_ENTER_NORMAL_MODE, "Exit visual mode"}
};
int visual_keymap_length = 1;

Map insert_keymap[] = {
    {'\x1b', ACTION_ENTER_NORMAL_MODE, "Exit insert mode"}
};
int insert_keymap_length = 1;

struct vector* normal_mode_keymap_vector;
struct vector* insert_mode_keymap_vector;
struct vector* visual_mode_keymap_vector;

struct vector* currKeymap_vector;

void initKeymaps() {
    normal_mode_keymap_vector = vector_init(malloc(sizeof(struct vector)), sizeof(Map));
    insert_mode_keymap_vector = vector_init(malloc(sizeof(struct vector)), sizeof(Map));
    visual_mode_keymap_vector = vector_init(malloc(sizeof(struct vector)), sizeof(Map));

    for (int i = 0; i < normal_keymap_length; i++) {
        vector_add(normal_mode_keymap_vector, moveMapToHeap(normal_keymap[i]));
    }
    for (int i = 0; i < insert_keymap_length; i++) {
        vector_add(insert_mode_keymap_vector, moveMapToHeap(insert_keymap[i]));
    }
    for (int i = 0; i < visual_keymap_length; i++) {
        vector_add(visual_mode_keymap_vector, moveMapToHeap(visual_keymap[i]));
    }
}

EditorAction keyMapLookup(int key) {
    for (int i = 0; i < currKeymap_vector->num_elements; i++) {
        Map* map = (Map *)vector_get(currKeymap_vector, i);
        if (key == map->key) return map->action;
    }
    return ACTION_UNKOWN;
}


EditorAction getEditorActionFromKey(EditorMode mode, int key) {
    switch (mode) {
        case EDITOR_NORMAL_MODE:
            currKeymap_vector = normal_mode_keymap_vector;
            break;
        case EDITOR_INSERT_MODE:
            currKeymap_vector = insert_mode_keymap_vector;
            break;
        case EDITOR_VISUAL_MODE:
            currKeymap_vector = visual_mode_keymap_vector;
            break;
        case EDITOR_COMMAND_MODE:
            currKeymap_vector = NULL;
            break;
    }
    return keyMapLookup(key);
}
