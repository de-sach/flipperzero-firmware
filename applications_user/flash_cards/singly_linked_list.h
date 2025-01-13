#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void* data;
    void* next;
} singly_linked_list_t;

typedef struct {
    size_t count;
    singly_linked_list_t* data;
} singly_linked_list_head_t;

void singly_linked_list_init(singly_linked_list_head_t* list);

void singly_linked_list_add(singly_linked_list_head_t* list, singly_linked_list_t* data);

void* signly_linked_list_get_data_at_index(singly_linked_list_head_t* list, size_t index);
