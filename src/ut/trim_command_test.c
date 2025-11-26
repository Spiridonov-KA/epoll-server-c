#include "../string_utilities.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct test_case {
    const char *input;
    const char *expected;
};

void run_test(const struct test_case *tc, size_t test_num) {
    char buffer[256];
    size_t input_len = strlen(tc->input);
    
    if (input_len >= sizeof(buffer)) {
        fprintf(stderr, "TEST #%zu FAILED: Input too long\n", test_num);
        exit(1);
    }
    
    strncpy(buffer, tc->input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    size_t result_size;
    char *result = trim_string(buffer, strlen(buffer), &result_size);

    if (!result) {
        fprintf(stderr, "TEST #%zu FAILED: Memory allocation failed\n", test_num);
    }

    bool is_equal = (strcmp(result, tc->expected) == 0);
    
    if (!is_equal) {
        printf("TEST #%zu FAILED\n", test_num);
        printf("Input:    \"%s\"\n", tc->input);
        printf("Expected: \"%s\"\n", tc->expected);
        printf("Got:      \"%s\"\n", result);
    }
    free(result);
#if 0
    printf("TEST #%zu PASSED: \"%s\" -> \"%s\"\n", 
           test_num, tc->input, result);
#endif
}

int main() {
    const struct test_case tests[] = {
        {"time", "time"},
        {" time", "time"},
        {"    time", "time"},
        {"time ", "time"},
        {"time    ", "time"},
        {" time ", "time"},
        {"   time     ", "time"},
        {"time\n", "time"},
        {"time\r\n", "time"},
        {"  time\n\r", "time"},
        {"hello world", "hello world"},
        {"  \t\n\r  trim me  \r\n\t  ", "trim me"},
        {"", ""},
        {"   ", ""},
        {"\n\r\t", ""},
        {"time with spaces", "time with spaces"},
        {"time\r\ntime", "time\r\ntime"},
        {"   time\r\ntime ", "time\r\ntime"},
    };
    
    size_t num_tests = sizeof(tests) / sizeof(tests[0]);
    
    for (size_t i = 0; i < num_tests; ++i) {
        run_test(&tests[i], i);
    }
}