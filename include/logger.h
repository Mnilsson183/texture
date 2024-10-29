#include <stdio.h>

#ifndef LOGGER_H
#define LOGGER_H

struct Logger {
	FILE* file;
};

struct Logger* initLogger(const char* filename);

#endif
