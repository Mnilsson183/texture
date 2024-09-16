#include "../include/vector.h"
#include <stdio.h>

void vectorTest(void) {
    struct vector* vec = vector_init(malloc(sizeof(struct vector)), sizeof(int));
    vector_add(vec, "Hello");
    printf("%s\n", (char*)vector_get(vec, 0));

}

int main(void) {
    vectorTest();
    return 0;
}
