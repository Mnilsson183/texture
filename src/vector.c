#include "../include/vector.h"
#include <stdlib.h>
#include <strings.h>


struct vector* vector_init_size(struct vector* self, int element_size, int initSize) {
    if (initSize <= 0) return NULL;
    if (element_size <= 0) return  NULL;
    self->element_size = element_size;
    self->max_elements = initSize;
    self->data = malloc(element_size*initSize);
    if (self->data == NULL) {
        free(self);
        return NULL;
    }
    self->num_elements = 0;
    return self;
}

struct vector* vector_init(struct vector* self, int element_size) {
    return vector_init_size(self, element_size, 30);
}

// will return NULL for bad resize values
void vector_resize(struct vector* self, int newSize) {
    if (newSize <= 0 || newSize < self->num_elements) return;
    self->max_elements = newSize;
    void* newData = malloc(self->element_size * newSize);
    memcpy(newData, self->data, self->element_size * self->num_elements);
    free(self->data);
    self->data = newData;
}

void vector_term(struct vector* self) {
    free(self->data);
    free(self);
}

void vector_term_ptrs(struct vector* self) {
    for (int i = 0; i < self->num_elements; i++) {
        free(vector_get(self, i));
    }
    vector_term(self);
}

// dont add 2 different types to my vector
void vector_add(struct vector* self, void* data) {
    if (self->num_elements + 1 == self->max_elements) {
        vector_resize(self, self->max_elements*2);
    }
    self->data[self->num_elements] = data;
    self->num_elements++;
}

void* vector_get(struct vector* self, int index) {
    if (index > self->num_elements) {
        return NULL;
    }
    return self->data[index];
}
