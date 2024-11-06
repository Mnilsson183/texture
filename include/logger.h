#include <stdio.h>

#ifndef LOGGER_H
#define LOGGER_H

struct Logger {
	FILE* file;
	void (*add)(struct Logger* logger, const char* val, ...);
	void (*warn)(struct Logger* logger, const char* val, ...);
	void (*error)(struct Logger* logger, const char* val, ...);
	void (*close)(struct Logger* logger);
};

struct Logger* initLogger(const char* filename);

#endif
