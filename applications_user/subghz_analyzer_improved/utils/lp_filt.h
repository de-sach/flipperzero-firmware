#pragma once

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t* buffer;
    size_t count;
    size_t index;
    size_t item_count;
} LowpassFilt;

LowpassFilt* lowpass_init(size_t item_count);
void lowpassfilt_add(LowpassFilt* filter, uint32_t item);
uint32_t lowpass_get(LowpassFilt* filter);
void lowpass_deinit(LowpassFilt* filter);