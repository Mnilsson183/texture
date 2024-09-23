#include "../include/editor.h"
#include "../include/highlight.h"
#include <stdlib.h>

int editorRowCxToRx(EditorRow *row, int cx){
    int rx = 0;
    int j;
    for(j = 0; j < cx; j++){
        if (row->chars[j] == '\t'){
            rx += (TEXTURE_TAB_STOP - 1) - (rx % TEXTURE_TAB_STOP);
        }
        rx++;
    }
    return rx;
}

int editorRowRxToCx(EditorRow *row, int rx){
    int cur_rx = 0;
    int cx;
    for(cx = 0; cx < row->size; cx++){
        if (row->chars[cx] == '\t'){
            cur_rx += (TEXTURE_TAB_STOP - 1) - (cur_rx % TEXTURE_TAB_STOP);
        }
        cur_rx++;
        if(cur_rx > rx){
            return cx;
        }
    }
    return cx;
}

void editorUpdateRow(EditorRow *row){
    int tabs = 0;
    int j;
    for (j = 0; j < row->size; j++){
        if (row->chars[j] == '\t'){
            tabs++;
        }
    }
    free(row->render);
    row->render = (char *)malloc(row->size + ( tabs * (TEXTURE_TAB_STOP - 1)) + 1);

    int tempLength = 0;
    for (j = 0; j < row->size; j++){
        if (row->chars[j] == '\t'){
            row->render[tempLength++] = ' ';
            while (tempLength % TEXTURE_TAB_STOP != 0){
                row->render[tempLength++] = ' ';
            }
        } else{
            row->render[tempLength++] = row->chars[j];
        }
    }
    row->render[tempLength] = '\0';
    row->renderSize = tempLength;

    editorUpdateSyntax(row);
}