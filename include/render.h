#ifndef TEXTURE_RENDER_H
#define TEXTURE_RENDER_H

#include "../include/editor.h"

void enableRawMode(struct Editor E);

struct AppendBuffer{
    // buffer to minimize write to terminal functions
    char *b;
    int len;
};

#define APPEND_INIT {NULL, 0}

void abAppend(struct AppendBuffer *ab, const char* s, int len);
void abFree(struct AppendBuffer *ab);
void editorRefreshScreen(struct Editor* E);

#endif
