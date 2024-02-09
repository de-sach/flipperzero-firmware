#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct MetroView* MetroView_t;
typedef uint8_t (*GetBpmCb)(void* ctx);
typedef void (*SetBpmCb)(uint8_t bpm, void* ctx);
typedef void (*SetActiveCb)(bool active, void* ctx);
typedef void (*SetEnabledCb)(bool enabled, void* ctx);
typedef void (*ScreenRequestCb)(void* ctx);

MetroView_t view_alloc(
    GetBpmCb get_bpm,
    SetBpmCb set_bpm,
    SetActiveCb set_active,
    SetEnabledCb set_enabled,
    ScreenRequestCb screen_req,
    void* ctx);

void view_run(MetroView_t view);
void view_free(MetroView_t view);
