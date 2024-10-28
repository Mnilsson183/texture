#ifndef ASSERT_H
#define ASSERT_H

int assertEquals(int one, int two, const char* msg);
int assertNotEquals(int one, int two, const char* msg);
int assertNULL(void* one, const char* msg);
int assertStringEqual(const char* s1, const char* s2, const char* msg);
int assertStringNotEqual(const char* s1, const char* s2, const char* msg);
int assertStringNEqual(const char* s1, const char* s2, int n, const char* msg);
int assertStringNNotEqual(const char* s1, const char* s2, int n, const char* msg);
int assertPtrEquals(const void* p1, const void* p2, const char* msg);

void allTestsPassing(const char* msg);
void printTestingSegment(const char* seg);

#endif
