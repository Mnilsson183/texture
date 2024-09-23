#include <stdlib.h>
#include <string.h>
#include "../include/keymap.h"
#include "../include/editor.h"
#include "../include/vector.h"


typedef struct {
    const char* key;
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
    {"q", ACTION_EDITOR_WINDOWS_EXIT, "Exit editor window"},
    {"i", ACTION_ENTER_INSERT_MODE, "Enter insert mode"},
    {"v", ACTION_ENTER_VISUAL_MODE, "Enter visual mode"},
    {STRING(ARROW_RIGHT), ACTION_MOVE_CURSOR_RIGHT, "Move cursor right"},
    {STRING(ARROW_LEFT), ACTION_MOVE_CURSOR_LEFT, "Move cursor left"},
    {STRING(ARROW_UP), ACTION_MOVE_CURSOR_UP, "Move cursor up"},
    {STRING(ARROW_DOWN), ACTION_MOVE_CURSOR_DOWN, "Move cursor down"},
    {STRING(CTRL_KEY('s')), ACTION_FS_SAVE_FILE, "Save the current buffer"},
    {"dd", ACTION_REMOVE_LINE, "Delete the current line"},
    {"l", ACTION_MOVE_CURSOR_RIGHT, "Move cursor right"},
    {"h", ACTION_MOVE_CURSOR_LEFT, "Move cursor left"},
    {"k", ACTION_MOVE_CURSOR_UP, "Move cursor up"},
    {"j", ACTION_MOVE_CURSOR_DOWN, "Move cursor down"},
};
#define NORMAL_LENGTH sizeof(normal_keymap) / sizeof(normal_keymap[0])

Map visual_keymap[] = {
    {"\x1b", ACTION_ENTER_NORMAL_MODE, "Exit visual mode"}
};
#define VISUAL_LENGTH sizeof(visual_keymap) / sizeof(visual_keymap[0])

Map insert_keymap[] = {
    {"\x1b", ACTION_ENTER_NORMAL_MODE, "Exit insert mode"},
    {"\r", ACTION_INSERT_NEWLINE, "Insert Newline"},
    {STRING(HOME_KEY), ACTION_MOVE_HOME_KEY, "Move with the home key"},
    {STRING(END_KEY), ACTION_MOVE_END_KEY, "Move with the end key"},
    {STRING(BACKSPACE), ACTION_REMOVE_BACKSPACE, "Backspace"},
    {STRING(CTRL_KEY('h')), ACTION_REMOVE_BACKSPACE, "Backspace"},
    {STRING(DEL_KEY), ACTION_REMOVE_DEL_KEY, "Del"},
    {STRING(ARROW_RIGHT), ACTION_MOVE_CURSOR_RIGHT, "Move cursor right"},
    {STRING(ARROW_LEFT), ACTION_MOVE_CURSOR_LEFT, "Move cursor left"},
    {STRING(ARROW_UP), ACTION_MOVE_CURSOR_UP, "Move cursor up"},
    {STRING(ARROW_DOWN), ACTION_MOVE_CURSOR_DOWN, "Move cursor down"},
};
#define INSERT_LENGTH sizeof(insert_keymap) / sizeof(insert_keymap[0])

struct vector* normal_mode_keymap_vector;
struct vector* insert_mode_keymap_vector;
struct vector* visual_mode_keymap_vector;

struct vector* currKeymap_vector;

void initKeymaps() {
    normal_mode_keymap_vector = vector_init(malloc(sizeof(struct vector)), sizeof(Map));
    insert_mode_keymap_vector = vector_init(malloc(sizeof(struct vector)), sizeof(Map));
    visual_mode_keymap_vector = vector_init(malloc(sizeof(struct vector)), sizeof(Map));

    for (int i = 0; i < NORMAL_LENGTH; i++) {
        vector_add(normal_mode_keymap_vector, moveMapToHeap(normal_keymap[i]));
    }
    for (int i = 0; i < INSERT_LENGTH; i++) {
        vector_add(insert_mode_keymap_vector, moveMapToHeap(insert_keymap[i]));
    }
    for (int i = 0; i < VISUAL_LENGTH; i++) {
        vector_add(visual_mode_keymap_vector, moveMapToHeap(visual_keymap[i]));
    }
}

EditorAction keyMapLookup(const char* key) {
    for (int i = 0; i < currKeymap_vector->num_elements; i++) {
        Map* map = (Map *)vector_get(currKeymap_vector, i);
        if (strcmp(key, map->key) == 0) return map->action;
    }
    for (int i = 0; i < currKeymap_vector->num_elements; i++) {
        Map* map = (Map *)vector_get(currKeymap_vector, i);
    }
    return ACTION_UNKOWN;
}


EditorAction getEditorActionFromKey(EditorMode mode, const char* key) {
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
