#ifndef VECTOR_H
#define VECTOR_H
typedef enum {
    OK,
    UNKNOWN,
    FAILED,
} VectorStatus;

struct vector {
    int element_size;
    int num_elements;
    int max_elements;
    VectorStatus status;
    void** data;
    void (*push)(struct vector* self, void* data);
    void* (*pop)(struct vector* self);
    void* (*get)(struct vector* self, int index);
};

struct vector* vector_init_size(struct vector* self, int element_size, int initSize);
struct vector* vector_init(struct vector* self, int element_size);
void vector_term(struct vector* self);
void vector_term_ptrs(struct vector* self);
void vector_add_data(struct vector* self, void* data[], int dataLength);
#endif
