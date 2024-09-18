#include "../include/editor.h"

#ifndef KEYMAP_H
#define KEYMAP_H

EditorAction getEditorActionFromKey(EditorMode mode, int key);
void initKeymaps(void);

#endif
