#include "list.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    int is_freed;
} test_item_t;

static int test_failed = 0;

#define TEST_ASSERT(condition, msg) do { \
    if (!(condition)) { \
        fprintf(stderr, "TEST FAILED: %s (line %d)\n  %s\n", __func__, __LINE__, msg); \
        test_failed = 1; \
        return; \
    } \
} while(0)

void test_create_list() {
    list_t* list = create_list();
    TEST_ASSERT(list != NULL, "List creation failed");
    TEST_ASSERT(list->start != NULL, "Start sentinel not created");
    TEST_ASSERT(list->end != NULL, "End sentinel not created");
    TEST_ASSERT(list->start->next == list->end, "Start->next != end");
    TEST_ASSERT(list->end->prev == list->start, "End->prev != start");
    destroy_list(list);
}

void test_add_single_node() {
    list_t* list = create_list();
    test_item_t* item = malloc(sizeof(test_item_t));
    item->id = 1;
    item->is_freed = 0;
    
    node_t* node = add_node(list, item);
    TEST_ASSERT(node != NULL, "Node addition failed");
    TEST_ASSERT(node->data == item, "Data pointer mismatch");
    TEST_ASSERT(list->start->next == node, "Start->next incorrect");
    TEST_ASSERT(node->prev == list->start, "Node->prev incorrect");
    TEST_ASSERT(node->next == list->end, "Node->next incorrect");
    TEST_ASSERT(list->end->prev == node, "End->prev incorrect");
    
    delete_node_and_data(node);
    destroy_list(list);
}

void test_add_multiple_nodes() {
    list_t* list = create_list();
    test_item_t items[3] = {{1, 0}, {2, 0}, {3, 0}};
    
    node_t* nodes[3];
    for (int i = 0; i < 3; i++) {
        nodes[i] = add_node(list, &items[i]);
        TEST_ASSERT(nodes[i] != NULL, "Node addition failed");
    }
    
    TEST_ASSERT(list->start->next == nodes[0], "First node position error");
    TEST_ASSERT(nodes[0]->next == nodes[1], "Second node position error");
    TEST_ASSERT(nodes[1]->next == nodes[2], "Third node position error");
    TEST_ASSERT(nodes[2]->next == list->end, "End sentinel position error");
    
    for (int i = 0; i < 3; i++) {
        delete_node(nodes[i]);
    }
    destroy_list(list);
}

void test_delete_node() {
    list_t* list = create_list();
    test_item_t* item = malloc(sizeof(test_item_t));
    
    node_t* node = add_node(list, item);
    delete_node(node);
    
    TEST_ASSERT(list->start->next == list->end, "List not empty after delete");
    TEST_ASSERT(list->end->prev == list->start, "List not empty after delete");
    
    free(item);
    destroy_list(list);
}

void test_destroy_list_with_data() {
    list_t* list = create_list();
    test_item_t* items[5];
    
    for (int i = 0; i < 5; i++) {
        items[i] = malloc(sizeof(test_item_t));
        items[i]->id = i;
        add_node(list, items[i]);
    }
    
    destroy_list_with_data(list);
}

void test_null_pointer_handling() {
    delete_node(NULL);
    delete_node_and_data(NULL);
    
    node_t* node = add_node(NULL, (void*)0x1234);
    TEST_ASSERT(node == NULL, "add_node should handle NULL list");
    
    destroy_list(NULL);
    destroy_list_with_data(NULL);
}

void test_empty_list_operations() {
    list_t* list = create_list();
    
    node_t fake_node = {.prev = list->start, .next = list->end};
    delete_node(&fake_node);
    
    destroy_list(list);
    
    destroy_list(NULL);
}

void run_all_tests() {
    test_create_list();
    test_add_single_node();
    test_add_multiple_nodes();
    test_delete_node();
    test_destroy_list_with_data();
    test_null_pointer_handling();
    
    if (test_failed) {
        fprintf(stderr, "\nSOME TESTS FAILED!\n");
        exit(1);
    }
    printf("\nALL TESTS PASSED!\n");
}

int main() {
    run_all_tests();
    return 0;
}