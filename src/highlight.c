#include "../include/highlight.h"

int editorSyntaxToColor(int highLight){
    switch (highLight)
    {
        case HL_COMMENT:
        case HL_MULTIPLE_LINE_COMMENT: return 36;
        case HL_KEYWORD1: return 33;
        case HL_KEYWORD2: return 32;
        case HL_NUMBER: return 31;
        case HL_STRING: return 35;
        case HL_MATCH: return 34;
        default: return 37;
    }
}