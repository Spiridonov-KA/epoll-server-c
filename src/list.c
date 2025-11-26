#include "list.h"
#include <stdlib.h>

list_t* create_list() {
    list_t* list = calloc(1, sizeof(list_t));
    if (!list) return NULL;

    node_t* start = calloc(1, sizeof(node_t));
    node_t* end = calloc(1, sizeof(node_t));
    if (!start || !end) {
        free(start);
        free(end);
        free(list);
        return NULL;
    }

    start->next = end;
    end->prev = start;
    list->start = start;
    list->end = end;
    return list;
}

void delete_node(node_t* node) {
    if (!node) return;
    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);
}

void delete_node_and_data(node_t* node) {
    if (!node) return;
    free(node->data);
    delete_node(node);
}

node_t* add_node(list_t* list, void* data) {
    if (!list) return NULL;
    
    node_t* new_node = calloc(1, sizeof(node_t));
    if (!new_node) return NULL;
    
    node_t* prev_end = list->end->prev;
    
    new_node->data = data;
    new_node->prev = prev_end;
    new_node->next = list->end;
    
    prev_end->next = new_node;
    list->end->prev = new_node;
    
    return new_node;
}

void destroy_list_with_data(list_t* list) {
    if (!list) return;
    
    node_t* current = list->start->next;
    while (current != list->end) {
        node_t* next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    
    free(list->start);
    free(list->end);
    free(list);
}

void destroy_list(list_t* list) {
    if (!list) return;
    
    node_t* current = list->start->next;
    while (current != list->end) {
        node_t* next = current->next;
        free(current);
        current = next;
    }
    
    free(list->start);
    free(list->end);
    free(list);
}