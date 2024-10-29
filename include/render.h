#include "../include/editor.h"

void enableRawMode(struct Editor E);

struct AppendBuffer{
    // buffer to minimize write to terminal functions
    char *b;
    int len;
};

void abAppend(struct AppendBuffer *ab, const char* s, int len);
void abFree(struct AppendBuffer *ab);
