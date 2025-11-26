#pragma once

typedef struct node_t {
    void *data;
    struct node_t* prev, *next;
} node_t;

typedef struct list_t {
    node_t* start;
    node_t* end;
} list_t;

list_t* create_list();

void delete_node(node_t* node);
void delete_node_and_data(node_t* node);

node_t* add_node(list_t* list, void* data);

void destroy_list_with_data(list_t* list);
void destroy_list(list_t* list);