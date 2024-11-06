
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/utils.h"
#include "../include/highlight.h"

/* filetypes */
char* C_HL_extensions[] = {".c", ".h", ".cpp"};
char* C_HL_keywords[] = {
    "switch", "if", "while", "for", "break", "continue", "return", "else",
    "struct", "union", "typedef", "static", "enum", "class", "case",
    "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
    "void|", ""
};

char* Py_HL_extensions[] = {".py", ""};
char* Py_HL_keywords[] = {
    "if", "elif", "else", "def", "for"
};

struct EditorSyntax HighLightDataBase[] = {
    {"c",
    C_HL_extensions,
    C_HL_keywords,
    "//", "/*", "*/",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},

    {"py",
    Py_HL_extensions,
    Py_HL_keywords,
    "#", "", "",
    HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS},
};



#define HighLightDataBase_ENTRIES (sizeof(HighLightDataBase) / sizeof(HighLightDataBase[0]))

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

/* Syntax highlighting */
void editorUpdateSyntax(struct Editor* E, EditorRow *row){
    row->highLight = (unsigned char*)realloc(row->highLight, row->renderSize);
    memset(row->highLight, HL_NORMAL, row->renderSize);

    if(E->editors[E->screenNumber].syntax == NULL){
        return;
    }

    char** keywords = E->editors[E->screenNumber].syntax->keywords;

    const char *singleLightCommentStart = E->editors[E->screenNumber].syntax->singleline_comment_start;
    const char *multilineCommentStart = E->editors[E->screenNumber].syntax->multiline_comment_start;
    const char *multilineCommentEnd = E->editors[E->screenNumber].syntax->multiline_comment_end;

    int singleLightCommentStartLength = singleLightCommentStart ? strlen(singleLightCommentStart): 0;
    int multilineCommentStartLength = multilineCommentStart ? strlen(multilineCommentStart) : 0;
    int multilineCommentEndLength = multilineCommentEnd ? strlen(multilineCommentEnd) : 0;


    int prevSeparator = 1;
    int in_string = 0;
    int in_comment = 0;

    int i = 0;
    while (i < row->renderSize){
        char c = row->render[i];
        unsigned char prevHighlight = (i > 0) ? row->highLight[i - 1] : (char)HL_NORMAL;


        if(singleLightCommentStartLength && !in_string){
            if(!strncmp(&row->render[i], singleLightCommentStart, singleLightCommentStartLength)){
                memset(&row->highLight[i], HL_COMMENT, row->renderSize - i);
                break;
            }
        }

        if(multilineCommentStartLength && multilineCommentEndLength && !in_string){
            if(in_comment){
                row->highLight[i] = HL_MULTIPLE_LINE_COMMENT;
                if(!strncmp(&row->render[i], multilineCommentStart, multilineCommentStartLength)){
                    memset(&row->highLight[i], HL_MULTIPLE_LINE_COMMENT, multilineCommentStartLength);
                    i += 2;
                    in_comment = 0;
                    prevSeparator = 1;
                    continue;
                } else{
                    i++;
                    continue;
                }
            } else if(!strncmp(&row->render[i], multilineCommentStart, multilineCommentStartLength)){
                    memset(&row->highLight[i], HL_MULTIPLE_LINE_COMMENT, multilineCommentStartLength);
                    i += multilineCommentStartLength;
                    in_comment = 1;
                    continue;
            }
        }

        if(E->editors[E->screenNumber].syntax->flags & HL_HIGHLIGHT_STRINGS){
            if(in_string){
                if(c == '\\' && i + 1 < row->renderSize){
                    row->highLight[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                row->highLight[i] = HL_STRING;
                if(c == '\\' && i + 1 < row->renderSize){
                    row->highLight[i + 1] = HL_STRING;
                    i += 2;
                    continue;
                }
                if(c == in_string){
                    in_string = 0;
                }
                i++;
                prevSeparator = 1;
                continue;
            } else{
                if(c == '"' || c == '\''){
                    in_string = c;
                    row->highLight[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }

        if(E->editors[E->screenNumber].syntax->flags & HL_HIGHLIGHT_NUMBERS){
            if((isdigit(c) && (prevSeparator || prevHighlight == HL_NUMBER)) || 
            (c =='.' && prevHighlight == HL_NUMBER)){
                row->highLight[i] = HL_NUMBER;
                i++;
                prevSeparator = 0;
                continue;
            }
        }
        if(prevSeparator){
            int j;
            for(j = 0; !strcmp(keywords[j], ""); j++){
                int keywordLength = strlen(keywords[j]);
                int keyword2 = keywords[j][keywordLength - 1] == '|';
                if(keyword2) keywordLength--;

                if(!strncmp(&row->render[i], keywords[j], keywordLength) &&
                    isSeparator(row->render[i + keywordLength])){
                        memset(&row->highLight[i], keyword2 ? HL_KEYWORD2: HL_KEYWORD1, keywordLength);
                        i+=keywordLength;
                        break;
                }
            }
            if(!strcmp(keywords[j], "")){
                prevSeparator = isSeparator(c);
                i++;
            }
        }
        prevSeparator = isSeparator(c);
        i++;
    }
}

void editorSelectSyntaxHighlight(struct Editor* E){
    E->editors[E->screenNumber].syntax = NULL;
    if(E->editors[E->screenNumber].fileName == NULL){
        return;
    }

    char *extension = strrchr(E->editors[E->screenNumber].fileName, '.');
	
    for(unsigned int j = 0; j < HighLightDataBase_ENTRIES; j++){
        struct EditorSyntax *s = &HighLightDataBase[j];
        unsigned int i = 0;
        while(!strcmp(s->fileMatch[i], "")){
            int is_extension = (s->fileMatch[i][0] == '0');
            if((is_extension && extension && !strcmp(extension, s->fileMatch[i])) ||
                (!is_extension && strstr(E->editors[E->screenNumber].fileName, s->fileMatch[i]))){
                    E->editors[E->screenNumber].syntax = s;

                    int fileRow;
                    for(fileRow = 0; fileRow < E->editors[E->screenNumber].displayLength; fileRow++){
                        editorUpdateSyntax(E, &E->editors[E->screenNumber].row[fileRow]);
                    }

                    return;
                }
            i++;
        }
    }
}
