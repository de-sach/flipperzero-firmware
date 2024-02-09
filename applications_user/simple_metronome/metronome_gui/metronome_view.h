#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct MetroView* MetroView_t;
typedef uint8_t (*GetBpmCb)(void* ctx);
typedef void (*SetBpmCb)(uint8_t bpm, void* ctx);
typedef void (*SetActiveCb)(bool active, void* ctx);

MetroView_t view_alloc(GetBpmCb get_bpm, SetBpmCb set_bpm, SetActiveCb set_active, void* ctx);
void view_run(MetroView_t view);
void view_free(MetroView_t view);