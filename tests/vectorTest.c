#include "../include/vector.h"
#include "../include/assert.h"
#include <stdlib.h>
#include "tests.h"

void vector_initTest() {
    struct vector* vec = vector_init(malloc(sizeof(struct vector)), sizeof(int));
    assertNotNULL(vec, "vector init");
    vector_term(vec);
}

void vector_getTest() {
    struct vector* vec = vector_init(malloc(sizeof(struct vector)), sizeof(int));
    assertNULL(vec->get(vec, -1), "negative index get");
    assertNULL(vec->get(vec, 100), "out of bounds index get");
}

//void vectorTest(void) {
//    struct vector* vec = vector_init(malloc(sizeof(struct vector)), sizeof(int));
//    int i = 0;
//    int j = 1;
//    int f = 99;
//    vector_push(vec, &i);
//    vector_push(vec, &j);
//    vector_push(vec, &f);
//    assertEquals(*(int*)vector_get(vec, 0), i, "Getting element from vector");
//    assertEquals(*(int*)vector_get(vec, 1), j, "Getting element from vector");
//    assertEquals(*(int*)vector_get(vec, 2), f, "Getting element from vector");
//    assertNULL(vector_get(vec, 4), "Can't get undefinded value from vector");
//    assertNULL(vector_get(vec, -1), "Can't get negative value from vector");
//    assertEquals(vec->num_elements, 3, "Vector doesnt know how long it is");
//    allTestsPassing("Vector");
//}

void run_vector_tests() {
    vector_initTest();
    return;
}
