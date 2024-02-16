
#include "lp_filt.h"

#include <furi.h>

LowpassFilt* lowpass_init(size_t item_count) {
    if(item_count == 0) {
        return NULL;
    }
    LowpassFilt* filter = malloc(sizeof(LowpassFilt));
    if(filter == NULL) {
        return NULL;
    }
    filter->buffer = (uint32_t*)calloc(sizeof(uint32_t), item_count);
    if(filter->buffer == NULL) {
        free(filter);
        return NULL;
    }
    filter->count = item_count;
    filter->index = 0u;
    filter->item_count = 0u;
    return filter;
}

void lowpassfilt_add(LowpassFilt* filter, uint32_t item) {
    furi_assert((filter != NULL) && (filter->buffer != NULL));
    filter->buffer[filter->index] = item;
    filter->item_count++;
    filter->index++;
    if(filter->index >= filter->count) {
        filter->index = 0u;
    }
}

uint32_t lowpass_get(LowpassFilt* filter) {
    furi_assert(filter && filter->buffer);
    uint64_t total = 0u;
    size_t max = (filter->item_count <= filter->count) ? filter->item_count : filter->count;
    for(size_t index = 0u; index < max; ++index) {
        total += (uint64_t)filter->buffer[index];
    }
    return (uint32_t)(total / max);
}

void lowpass_deinit(LowpassFilt* filter) {
    if(filter) {
        filter->count = 0u;
        filter->index = 0u;
        filter->item_count = 0u;
        if(filter->buffer) {
            free(filter->buffer);
        }
        free(filter);
    }
}