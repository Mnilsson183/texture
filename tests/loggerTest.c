#include "../include/logger.h"
#include "../include/assert.h"
#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void logger_addTest() {
    remove("test.out");
    struct Logger* logger = initLogger("test.out");
    const char* str = "five is 5";
    logger->add(logger, str);
    logger->close(logger);

    FILE* f = fopen("test.out", "r");
    char buf[30];
    fgets(buf, strlen(str)+1, f);
    assertStringEqual(buf, str, "Log a string into a file");
    fclose(f);
    remove("test.out");

    logger = initLogger("test.out");
    logger->add(logger, "five is %d", 5);
    logger->close(logger);

    f = fopen("test.out", "r");
    fgets(buf, 10, f);
    assertStringEqual(buf, "five is 5", "Log a string into a file");
    fclose(f);
    remove("test.out");
}

void run_logger_tests() {
    printTestingSegment("Logger");
    logger_addTest();
    return;
}
