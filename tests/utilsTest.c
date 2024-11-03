#include "../include/utils.h"
#include "../include/assert.h"
#include "tests.h"

void minTest() {
    assertEquals(min(4, 2), 2, "min");
    assertEquals(min(2, 2), 2, "min");
    assertEquals(min(2, 4), 2, "min");
    assertEquals(min(2, -4), -4, "min");
}

void maxTest() {
    assertEquals(max(4, 2), 4, "max");
    assertEquals(max(2, 2), 2, "max");
    assertEquals(max(2, 4), 4, "max");
    assertEquals(max(2, -4), 2, "max");
    assertEquals(max(-2, -4), -2, "max");
}

void isSeparatorTest() {
    assertTrue(isSeparator(' '), "isSepatator");
    assertTrue(isSeparator('\0'), "isSepatator");
    assertTrue(isSeparator(','), "isSepatator");
    assertTrue(isSeparator('['), "isSepatator");
    assertFalse(isSeparator('2'), "isSeparator");
    assertFalse(isSeparator('a'), "isSeparator");
}

void run_utils_tests() {
    printTestingSegment("Utils tests");
    minTest();
    maxTest();
    isSeparatorTest();
    allTestsPassing("Utils");
    return;
}
