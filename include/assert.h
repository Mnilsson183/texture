#ifndef ASSERT_H
#define ASSERT_H

int assertEquals(int one, int two, const char* msg);
int assertNotEquals(int one, int two, const char* msg);

void allTestsPassing(const char* msg);

#endif
