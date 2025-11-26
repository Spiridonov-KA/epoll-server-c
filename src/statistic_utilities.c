#include "statistic_utilities.h"

static struct server_state* get_server_state_() {
    static struct server_state server = {.total_connections = 0, .current_connections = 0};
    return &server;
}

void add_connection() {
    struct server_state* server = get_server_state_();
    ++server->current_connections;
    ++server->total_connections;
}

void sub_connection() {
    struct server_state* server = get_server_state_();
    --server->current_connections;
}

struct server_state get_server_info() {
    struct server_state* server = get_server_state_();
    struct server_state copy_server = {.total_connections = server->total_connections, 
        .current_connections = server->current_connections};
    return copy_server;
}
