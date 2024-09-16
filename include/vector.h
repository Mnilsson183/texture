#include <stdlib.h>

typedef enum {
    OK,
    UNKNOWN,
    FAILED,
} VectorEnum;

struct vector {
    int element_size;
    int num_elements;
    int max_elements;
    VectorEnum status;
    void** data;
};

struct vector* vector_init_size(struct vector* self, int element_size, int initSize);
struct vector* vector_init(struct vector* self, int element_size);
int vector_size(struct vector* self);
void vector_term(struct vector* self);
void vector_term_ptrs(struct vector* self);
void vector_add(struct vector* self, void* data);
void* vector_get(struct vector* self, int index);
