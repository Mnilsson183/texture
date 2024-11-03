#include "tests.h"
#include <stdio.h>

int main() {
    printf("Running all tests...\n");

    run_dict_tests();
    run_editor_tests();
    run_highlight_tests();
    run_keymap_tests();
    run_logger_tests();
    run_render_tests();
    run_texture_tests();
    run_utils_tests();
    run_vector_tests();

    printf("All tests completed.\n");
    return 0;
}
