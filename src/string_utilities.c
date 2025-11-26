#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "string_utilities.h"

char* trim_string(char *cmd, size_t len, size_t *result_size) {
    if (result_size == NULL) {
        return NULL;
    }
    *result_size = 0;
    if (len == 0) {
        char *res = malloc(1);
        if (res) res[0] = '\0';
        return res;
    }
    
    while (len > 0 && isspace((unsigned char)cmd[len-1])) {
        len--;
    }
    
    size_t start = 0;
    while (start < len && isspace((unsigned char)cmd[start])) {
        start++;
    }
    
    size_t result_len = len - start;
    char *res = malloc(result_len + 1);
    
    if (!res) return NULL;
    
    memcpy(res, cmd + start, result_len);
    res[result_len] = '\0';
    *result_size = result_len;
    
    return res;
}