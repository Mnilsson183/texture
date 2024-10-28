#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_RED "\033[31m"
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"

int tests = 0;
int passed = 0;

void failedTest(const char* msg) {
    tests++;
    printf("%d/%d ", passed, tests);
    if (msg == NULL) {
        fprintf(stderr, COLOR_RED "Test failed: No message provided.\n" COLOR_RESET);
    } else {
        fprintf(stderr, COLOR_RED "Test failed: %s\n" COLOR_RESET, msg);
    }
}

void okTest(const char* msg) {
    tests++;
    passed++;
    printf("%d/%d ", passed, tests);
    if (msg == NULL) {
        fprintf(stdout, COLOR_GREEN "Test passed: No message provided.\n" COLOR_RESET);
    } else {
        fprintf(stdout, COLOR_GREEN "Test passed: %s\n" COLOR_RESET, msg);
    }
}

void printTestingSegment(const char* seg) {
    printf("---------------------%s---------------------\n", seg);
}

void allTestsPassing(const char* msg) {
    if (msg == NULL) {
        fprintf(stdout, COLOR_GREEN "All Test passed: No message provided.\n" COLOR_RESET);
    } else {
        fprintf(stdout, COLOR_GREEN "All Test passed: %s\n" COLOR_RESET, msg);
    }
}

void valueComparePtr(const void * p1, const void* p2) {
    printf("    - %d  vs  %d", p1, p2);
}

void valueCompareValue(const int i1, const int i2) {
    printf("    - %d  vs  %d", i1, i2);
}

void valueCompareString(const char* c_str1, const char* c_str2) {
    printf("    - |%s|  vs  |%s|", c_str1, c_str2);
}

void valueCompareNString(const char* c_str1, const char* c_str2, const int l1, const int l2) {

    printf("    -  |");
    for (int i = 0; i < l1; i++) {
        printf("%c", c_str1[i]);
    }
    printf("|  vs  |");
    for (int i = 0; i < l2; i++) {
        printf("%c", c_str2[i]);
    }
    printf("|\n");
}

int assertEquals(int one, int two, const char* msg) {
    if (one != two) {
        failedTest(msg);
        valueCompareValue(one, two);
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
        valueCompareValue(one, two);
        return 1;
    }
}

int assertNULL(void* one, const char* msg) {
    if (one != NULL) {
        failedTest(msg);
        return 1;
    } else {
        okTest(msg);
        return 0;
    }
}

int assertStringEqual(const char* s1, const char* s2, const char* msg) {
    if (strcmp(s1, s2) == 0) {
        okTest(msg);
        return 0;
    } else {
        failedTest(msg);
        valueCompareString(s1, s2);
        return 1;
    }
}

int assertStringNotEqual(const char* s1, const char* s2, const char* msg) {
    if (strcmp(s1, s2) == 0) {
        failedTest(msg);
        valueCompareString(s1, s2);
        return 1;
    } else {
        okTest(msg);
        return 0;
    }
}

int assertStringNEqual(const char* s1, const char* s2, int n, const char* msg) {
    if (strncmp(s1, s2, n) == 0) {
        okTest(msg);
        return 0;
    } else {
        failedTest(msg);
        valueCompareNString(s1, s2, n, n);
        return 1;
    }
}

int assertStringNNotEqual(const char* s1, const char* s2, int n, const char* msg) {
    if (strncmp(s1, s2, n) == 0) {
        failedTest(msg);
        valueCompareNString(s1, s2, n, n);
        return 1;
    } else {
        okTest(msg);
        return 0;
    }
}

int assertPtrEquals(const void* p1, const void* p2, const char* msg) {
    if (p1 == p2) {
        okTest(msg);
        return 0;
    } else {
        failedTest(msg);
        valueComparePtr(p1, p2);
        return 1;
    }
}
