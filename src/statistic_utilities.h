#pragma once
#include <stdio.h>

struct server_state {
    size_t total_connections;
    size_t current_connections;
};

struct server_state get_server_info();
void add_connection();
void sub_connection();
void close_connection(int socket);