#include "stdlib.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "dict.h"

struct Dictionary* createDictionary(void) {
    struct Dictionary *dict = (struct Dictionary *)malloc(sizeof(struct Dictionary));
    dict->nodesSize = NODE_SIZE;
    for (size_t i = 0; i < NODE_SIZE; i++) {
    }
    return dict;
}

void freeNode(struct DictNode *node, struct Dictionary *dict) {
    if (node == NULL) return;

    for (uint8_t i = 0; i < dict->nodesSize; i++) {
        freeNode(node->nodes[i], dict);
    }
    free(node);
}

void freeDictionary(struct Dictionary *dict){
    for (uint8_t i = 0; i < dict->nodesSize; i++) {
        freeNode(&dict->nodes[i], dict);
    }
}

enum CHARTYPE getType(char c) {
    if (c > 64 && c < 91) return UPPERCASE;
    if (c > 96 && c < 123) return LOWERCASE;
    if (c > 47 && c < 58) return DIGIT;
    else return OTHER;
}

size_t getHashed(char c) {
    enum CHARTYPE type = getType(c);
    switch (type) {
        case LOWERCASE: return c - 'a';
        case UPPERCASE: return c - 'A' + 26;
        case DIGIT: return  c - '0' + 52;
        case OTHER: // 62 total
            switch (c) {
                case '\'': return 63;
                case '_': return 64;
                default: return 65;
            }
    }
}

void addWord(struct Dictionary* dict, const char* word) {
    if (dict == NULL || word == NULL || *word == '\0') return;
    struct DictNode *node = &dict->nodes[getHashed(word[0])];
    size_t word_len = strlen(word);
    for (size_t i = 0; i < word_len; i++) {
        int index = getHashed(word[i]);
        printf("%s\n", word);

        if (node->nodes[index] == NULL) {
            printf("making new node\n");
            node->nodes[index] = (struct DictNode *)malloc(sizeof(struct DictNode));
            if (node->nodes[index] == NULL){
                fprintf(stderr, "Error in allocation\n");
                return;
            }
            for (size_t child = 0; child < NODE_SIZE; child++) {
                node->nodes[index]->nodes[child] = NULL;
            }
        }
        node = node->nodes[index];
    }
    node->freq++;
    node->isWord = true;
}

bool getIsWord(struct Dictionary *dict, const char *word) {
    if ( dict == NULL || word == NULL || *word == '\0') return false;

    struct DictNode *node = &dict->nodes[getHashed(word[0])];
    size_t word_len = strlen(word);
    for (size_t i = 0; i < word_len; i++) {
        int index = getHashed(word[i]);
        if (node->nodes[index] == NULL) return false;
        node = node->nodes[index];
    }
    return node->isWord;
}

int32_t getFreq(struct Dictionary *dict, const char *word) {
    if ( dict == NULL || word == NULL || *word == '\0') return false;

    struct DictNode *node = &dict->nodes[getHashed(word[0])];
    size_t word_len = strlen(word);
    for (size_t i = 0; i < word_len; i++) {
        int index = getHashed(word[i]);
        if (node->nodes[index] == NULL) return false;
        node = node->nodes[index];
    }
    return node->freq;
}

// should still add tree shaking of nodes
void removeWord(struct Dictionary *dict, const char *word) {
    if ( dict == NULL || word == NULL || *word == '\0') return;

    struct DictNode *node = &dict->nodes[getHashed(word[0])];
    size_t word_len = strlen(word);
    for (size_t i = 0; i < word_len; i++) {
        int index = getHashed(word[i]);
        if (node->nodes[index] == NULL) return;
        node = node->nodes[index];
    }
    node->freq = 0;
    node->isWord = false;
}
