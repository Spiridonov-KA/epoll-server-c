#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <time.h>
#include <errno.h>

#include "utilities.h"

int set_connection() {
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in servAdr;
    memset(&servAdr, 0, sizeof(servAdr));
    char *servIp = "127.0.0.1";
    servAdr.sin_family = AF_INET;
    servAdr.sin_port = htons(33333);
    {
        int val = inet_pton(AF_INET, servIp, &servAdr.sin_addr.s_addr);
        if (val == 0) {
            die_with_user_message("inet_pton() failed", "invalid address string");
        } else if (val < 0) {
            die_with_system_message("inet_pton() failed");
        }
    }
    if (connect(fd, (struct sockaddr*) &servAdr, sizeof(servAdr)) < 0) {
        die_with_system_message("connect() failed");
    }

    return fd;
}

int main() {
    signal(SIGPIPE, SIG_IGN);

    int fd = set_connection();

    int max_len = 100;
    char* str = (char *)malloc(sizeof(char) * max_len);

    for (int i = 0;; ++i) {
        time_t result = time(NULL);
        sprintf(str, "The current time is %s(%jd seconds since the Epoch)\n",
                asctime(gmtime(&result)), (intmax_t)result);
        ssize_t n = write(fd, str, max_len);
        if (n < 0) {
            if (errno == EPIPE) {
                printf("Server closed connection. Reconnecting...\n");
                close(fd);
                fd = set_connection();
            } else {
                perror("write");
                exit(1);
            }
        }
        usleep(100);
    }
}