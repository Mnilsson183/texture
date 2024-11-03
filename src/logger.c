#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../include/logger.h"

void logger_add(struct Logger* logger, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(logger->file, format, args);
    fprintf(logger->file, "\n");
    fflush(logger->file);
    va_end(args);
}

void closeLogger(struct Logger* logger) {
    fflush(logger->file);
    fclose(logger->file);
    free(logger);
}

struct Logger* initLogger(const char* filename) {
    struct Logger* logger = malloc(sizeof(struct Logger));
    if (logger == NULL) return NULL;
    logger->file = fopen(filename, "a");
    logger->add = &logger_add;
    logger->close = &closeLogger;
    return logger;
}
