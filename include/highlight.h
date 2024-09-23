#include <stdlib.h>
#include "../include/editor.h"

#ifndef HIGHLIGHT_H
#define HIGHLISHT_H

#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

void editorUpdateSyntax(EditorRow *row);

#endif