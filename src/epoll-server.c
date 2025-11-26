#include <stddef.h>
#include <sys/types.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <signal.h>

#include "string_utilities.h"
#include "statistic_utilities.h"
#include "utilities.h"
#include "list.h"

#define MAXPENDING 5
#define MAX_EVENTS 10
#define PORT 33333
#define BUFSIZ_ANSWER 1024

volatile sig_atomic_t server_running = 1;

typedef  struct {
    int fd;
} socket_info_t;

typedef struct {
    int fd;
    node_t* node;
} client_info_t;

int create_listen_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

    if (sockfd < 0) {
        die_with_system_message("socket() failed");
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        die_with_system_message("setsockopt() failed");
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        die_with_system_message("bind() failed");
    }

    if (listen(sockfd, MAXPENDING) < 0) {
        die_with_system_message("listen() failed");
    }

    return sockfd;
}

int create_epoll_socket(int listen_socket) {
    int epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        close(listen_socket);
        die_with_system_message("epoll_create1() failed");
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    client_info_t *ci = calloc(1, sizeof(client_info_t));
    ci->fd = listen_socket;
    ci->node = NULL;
    event.data.ptr = ci;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_socket, &event) < 0) {
        close(listen_socket);
        close(epoll_fd);
        die_with_system_message("epoll_ctl() failed");
    }
    
    return epoll_fd;
}

void handle_event_from_listen_socket(int epoll_socket, int listen_socket, list_t* client_sockets) {
    struct epoll_event event;
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept4(listen_socket, 
                                (struct sockaddr *)&client_addr, 
                                &client_len, 
                                SOCK_NONBLOCK);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("accept4 failed");
            break;
        }

        event.events = EPOLLIN | EPOLLET | EPOLLHUP;
        client_info_t *ci = calloc(1, sizeof(client_info_t));
        if (!ci) {
            perror("calloc client_info_t");
            close(client_fd);
            continue;
        }
        ci->fd = client_fd;
        ci->node = add_node(client_sockets, ci);

        event.data.ptr = ci;

        if (epoll_ctl(epoll_socket, EPOLL_CTL_ADD, client_fd, &event) < 0) {
            perror("epoll_ctl client failed");
            delete_node(ci->node);
            free(ci);
            close(client_fd);
            continue;
        }
        char ip_str[INET_ADDRSTRLEN];
        if (inet_ntop(AF_INET, &client_addr.sin_addr, ip_str, sizeof(ip_str)) == NULL) {
            perror("inet_ntop failed");
            strncpy(ip_str, "invalid", sizeof(ip_str));
        }
        add_connection();
        printf("New connection: fd=%d, ip=%s, port=%d\n",
            client_fd, ip_str, ntohs(client_addr.sin_port));
    }
}

ssize_t read_data_in_buffer(char *buffer, int buffer_size, int client_socket) {
    size_t total_size = 0;
    ssize_t count = 0;
    while (total_size + 1 < buffer_size && (count = read(client_socket, buffer + total_size, buffer_size - total_size - 1)) > 0) {
        printf("[MSG%d] %.*s\n", client_socket, (int)count, buffer + total_size);
        total_size += count;
    }
    if (count == -1 && errno != EAGAIN) {
        perror("read() failed");
        return -1;
    } else if (count == 0) {
        printf("Disconnected fd: %d\n", client_socket);
        return 0;
    }
    buffer[total_size] = '\0';
    return total_size;
}

bool is_command(char *str) {
    if (str == NULL) {
        return false;
    }
    if (str[0] == '/') {
        return true;
    }
    return false;
}

void shutdown_server(int epoll_fd, int listen_socket, list_t* client_sockets) {
    node_t* current = client_sockets->start->next;
    while (current != client_sockets->end) {
        client_info_t* client_info = current->data;
        node_t* next = current->next;
        
        const char* msg = "Server shutting down...\n";
        write(client_info->fd, msg, strlen(msg));

        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_info->fd, NULL);
        close(client_info->fd);

        delete_node(current);
        current = next;
    }

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, listen_socket, NULL);
    close(listen_socket);
    close(epoll_fd);

    server_running = 0;
}


void prepare_answer_for_command(char *command, size_t command_len, char *buffer, size_t buffer_len) {
    size_t clear_command_len;
    char *clear_command = trim_string(command, command_len, &clear_command_len);
    if (clear_command == NULL) {
        sprintf(buffer, "An error occurred while processing the command\n");
    }
    if (strcmp(clear_command, "/time") == 0) {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);
        strftime(buffer, buffer_len, "%Y-%m-%d %H:%M:%S\n", &tm_now);
    } else if (strcmp(clear_command, "/stats") == 0) {
        struct server_state server_info = get_server_info();
        sprintf(buffer, "total_connections = %zd; current_connections = %zd\n", 
            server_info.total_connections, server_info.current_connections);
    } else if (strcmp(clear_command, "/shutdown") == 0) {
        sprintf(buffer, "shutdown");
    } else {
        sprintf(buffer, "Unknown command\n");
    }
}

void handle_client_socket(int epoll_socket, int listen_socket, int socket, node_t* node_ptr, list_t* client_sockets) {
    char buffer[BUFSIZ];
    ssize_t actual_len = read_data_in_buffer(buffer, BUFSIZ, socket);
    if (actual_len <= 0) {
        epoll_ctl(epoll_socket, EPOLL_CTL_DEL, socket, NULL);
        delete_node_and_data(node_ptr);
        close(socket);
        sub_connection();
        return;
    }
    if (is_command(buffer)) {
        char answer_buffer[BUFSIZ_ANSWER];
        prepare_answer_for_command(buffer, (size_t)actual_len, answer_buffer, BUFSIZ_ANSWER - 1);
        if (strcmp(answer_buffer, "shutdown") == 0) {
            shutdown_server(epoll_socket, listen_socket, client_sockets);
            return;
        }
        write(socket, answer_buffer, strlen(answer_buffer));
    } else {
        write(socket, buffer, actual_len);
    }
    printf("BUFFER: %s\n", buffer);
}

void handle_events(int epoll_socket, int listen_socket, list_t* client_sockets,
                   struct epoll_event *events, int nfds) {
    for (int i = 0; i < nfds; ++i) {

        int socket = ((client_info_t*)events[i].data.ptr)->fd;
        node_t* node_ptr = ((client_info_t*)events[i].data.ptr)->node;
        uint32_t event = events[i].events;

        if (event & (EPOLLERR | EPOLLHUP)) {
            perror("epoll error");
            epoll_ctl(epoll_socket, EPOLL_CTL_DEL, socket, NULL);
            close(socket);
        } else if (event & EPOLLRDHUP) {
            printf("Client closed (RDHUP): %d\n", socket);
            close(socket);
        } else if (socket == listen_socket) {
            handle_event_from_listen_socket(epoll_socket, socket, client_sockets);
        } else if (event & EPOLLIN) {
            handle_client_socket(epoll_socket, listen_socket, socket, node_ptr, client_sockets);
        } else {
            printf("wait for updates:)\n");
        }
    }
}


void run_server() {
    int listen_socket = create_listen_socket();

    int epoll_socket = create_epoll_socket(listen_socket);

    list_t* client_sockets = create_list();

    struct epoll_event events[MAX_EVENTS];
    while (server_running) {
        int nfds = epoll_wait(epoll_socket, events, MAX_EVENTS, -1);

        handle_events(epoll_socket, listen_socket, client_sockets, events, nfds);
    }

    destroy_list_with_data(client_sockets);
}

int main() {
    run_server();
}