#include <stdio.h>
#include <stdlib.h>

#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"

void failedTest(const char* msg) {
    if (msg == NULL) {
        fprintf(stderr, COLOR_RED "Test failed: No message provided.\n" COLOR_RESET);
    } else {
        fprintf(stderr, COLOR_RED "Test failed: %s\n" COLOR_RESET, msg);
    }
    exit(1);
}

void okTest(const char* msg) {
    if (msg == NULL) {
        fprintf(stdout, COLOR_GREEN "Test passed: No message provided.\n" COLOR_RESET);
    } else {
        fprintf(stdout, COLOR_GREEN "Test passed: %s\n" COLOR_RESET, msg);
    }
}

void allTestsPassing(const char* msg) {
    if (msg == NULL) {
        fprintf(stdout, COLOR_GREEN "All Test passed: No message provided.\n" COLOR_RESET);
    } else {
        fprintf(stdout, COLOR_GREEN "All Test passed: %s\n" COLOR_RESET, msg);
    }
}

int assertEquals(int one, int two, const char* msg) {
    if (one != two) {
        failedTest(msg);
        return 0;
    } else {
        okTest(msg);
        return 1;
    }
}

int assertNotEquals(int one, int two, const char* msg) {
    if (one == two) {
        okTest(msg);
        return 0;
    } else {
        failedTest(msg);
        return 1;
    }
}
