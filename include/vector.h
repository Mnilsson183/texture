#include <stdlib.h>

struct vector {
    int element_size;
    int num_elements;
    int max_elements;
    void** data;
};

struct vector* vector_init_size(struct vector* self, int element_size, int initSize);
struct vector* vector_init(struct vector* self, int element_size);
void vector_term(struct vector* self);
void vector_add(struct vector* self, void* data);
void* vector_get(struct vector* self, int index);
