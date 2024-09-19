#include "../include/editor.h"

#ifndef KEYMAP_H
#define KEYMAP_H

typedef enum {
	ACTION_UNKOWN,
	ACTION_IGNORE,
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
	ACTION_EXECUTE_DIR,
	
	ACTION_INSERT_NEWLINE,

	ACTION_REMOVE_BACKSPACE,
	ACTION_REMOVE_DEL_KEY,

	ACTION_MOVE_HOME_KEY,
	ACTION_MOVE_END_KEY,
} EditorAction;

#define CTRL_KEY(key) ((key) & 0x1f)

EditorAction getEditorActionFromKey(EditorMode mode, int key);
void initKeymaps(void);

#endif
