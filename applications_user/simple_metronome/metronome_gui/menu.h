#pragma once

#include <stdbool.h>
#include "metronome.h"

typedef struct metronomeMenu* metronomeMenu_t;
typedef void (*ScreenRequestCb)(void* ctx);
typedef void (*NotificationTypeCb)(bool status, NotificationType notification, void* ctx);
typedef bool (*NotificationGetCb)(NotificationType notification, void* ctx);
typedef void (*RythmCb)(RythmType rythm, void* ctx);

metronomeMenu_t menu_alloc(
    ScreenRequestCb screen_req,
    NotificationTypeCb notification_type_cb,
    NotificationGetCb notification_get_cb,
    RythmCb rythm_cb,
    void* cb_ctx);
void menu_run(metronomeMenu_t menu);
void menu_free(metronomeMenu_t menu);