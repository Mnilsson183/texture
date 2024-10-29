#include <stdio.h>
#include <stdlib.h>
#include "../include/logger.h"

struct Logger* initLogger(const char* filename) {
    struct Logger* logger = malloc(sizeof(struct Logger));
    if (logger == NULL) return NULL;
    logger->file = fopen(filename, "+a");
    return logger;
}
