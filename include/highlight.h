#include <stdlib.h>
#include "../include/editor.h"

#ifndef HIGHLIGHT_H
#define HIGHLISHT_H

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

enum editorHighlight {
    HL_NORMAL = 0,
    HL_COMMENT,
    HL_MULTIPLE_LINE_COMMENT,
    HL_KEYWORD1,
    HL_KEYWORD2,
    HL_STRING,
    HL_NUMBER,
    HL_MATCH
};

int editorSyntaxToColor(int highLight);
void editorUpdateSyntax(struct Editor* E, EditorRow *row);

#endif
