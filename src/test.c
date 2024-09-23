#include "../include/vector.h"
#include "../include/keymap.h"
#include "../include/assert.h"
#include "../include/editor.h"
#include <stdio.h>
#include <stdlib.h>

void vectorTest(void) {
    struct vector* vec = vector_init(malloc(sizeof(struct vector)), sizeof(int));
    int i = 0;
    int j = 1;
    int f = 99;
    vector_add(vec, &i);
    vector_add(vec, &j);
    vector_add(vec, &f);
    assertEquals(*(int*)vector_get(vec, 0), i, "Getting element from vector");
    assertEquals(*(int*)vector_get(vec, 1), j, "Getting element from vector");
    assertEquals(*(int*)vector_get(vec, 2), f, "Getting element from vector");
    assertNULL(vector_get(vec, 4), "Can't get undefinded value from vector");
    assertNULL(vector_get(vec, -1), "Can't get negative value from vector");
    assertEquals(vec->num_elements, 3, "Vector doesnt know how long it is");
    allTestsPassing("Vector");
}

void keymapTest(void) {
    initKeymaps();
    assertEquals(getEditorActionFromKey(EDITOR_NORMAL_MODE, "i"), ACTION_ENTER_INSERT_MODE, "Enter insert mode from normal");
    assertEquals(getEditorActionFromKey(EDITOR_NORMAL_MODE, "dd"), ACTION_REMOVE_LINE, "Delete a line from normal mode");
    assertEquals(getEditorActionFromKey(EDITOR_NORMAL_MODE, "h"), ACTION_MOVE_CURSOR_LEFT, "Move the cursor left with h");

    assertEquals(getEditorActionFromKey(EDITOR_VISUAL_MODE, "\x1b"), ACTION_ENTER_NORMAL_MODE, "Enter normal mode from visual");

    assertEquals(getEditorActionFromKey(EDITOR_INSERT_MODE, "\x1b"), ACTION_ENTER_NORMAL_MODE, "Enter normal mode from insert");
    assertEquals(getEditorActionFromKey(EDITOR_INSERT_MODE, STRING(BACKSPACE)), ACTION_REMOVE_BACKSPACE, "Backspace in insert mode");

    allTestsPassing("Keymap");
}

int main(void) {
    vectorTest();
    keymapTest();
    allTestsPassing("Project");
    return 0;
}
