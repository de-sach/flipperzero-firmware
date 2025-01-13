
#include "singly_linked_list.h"
#include <furi.h>

void singly_linked_list_init(singly_linked_list_head_t* list) {
    list->count = 0u;
    list->data = NULL;
}

void singly_linked_list_add(singly_linked_list_head_t* list, singly_linked_list_t* data) {
    if(data) {
        list->count++;
        if(list->data == NULL) {
            list->data = data;
        } else {
            singly_linked_list_t* next = list->data;
            furi_assert(next != NULL);
            while(next->next != NULL) {
                next = next->next;
            }
            next->next = data;
            data->next = NULL;
        }
    }
}

void* signly_linked_list_get_data_at_index(singly_linked_list_head_t* list, size_t index) {
    if(index >= list->count) {
        return NULL;
    }
    singly_linked_list_t* item = list->data;
    while(index > 0) {
        index--;
        if(item->next != NULL) {
            item = item->next;
        }
    }
    return item->data;
}
