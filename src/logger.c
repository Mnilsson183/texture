#include <stdio.h>
#include <stdlib.h>
#include "../include/logger.h"

FILE* logFile;

FILE* initLogFile(const char* filename) {
    if (filename == NULL) {
        return fopen("log.txt", "a");
    } else {
        return fopen(filename, "a");
    }
}

void logger_add(const char* log) {
    fputs(log, logFile);
}
