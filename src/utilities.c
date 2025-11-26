#include <stdio.h>
#include <stdlib.h>

#include "utilities.h"

void die_with_user_message(const char *msg, const char *detail) {
    fputs(msg, stderr);
    fputs(": ", stderr);
    fputs(detail, stderr);
    fputc('\n', stderr);
    exit(1);
}

void die_with_system_message(const char *msg) {
    perror(msg);
    exit(1);
}