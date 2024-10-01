#include <stdio.h>

#ifndef LOGGER_H
#define LOGGER_H

FILE* initLogFile(const char* filename);
void logger_add(const char* log);

#endif
