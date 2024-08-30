#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

const static int16_t NODE_SIZE = 65;

enum CHARTYPE {
	LOWERCASE,
	UPPERCASE,
	DIGIT,
	OTHER
};

struct DictNode {
	int32_t freq;
	struct DictNode* nodes[NODE_SIZE];
	bool isWord;
};


struct Dictionary {
	struct DictNode* nodes;
	int16_t nodesSize;
};

struct Dictionary* createDictionary(void);
void initDictionary(const char *filename);
void freeDictionary(struct Dictionary *dict);

void addWord(struct Dictionary *dict, const char *word);
bool getIsWord(struct Dictionary *dict, const char *word);
int32_t getFreq(struct Dictionary *dict, const char *word);
void removeWord(struct Dictionary *dict, const char *word);
