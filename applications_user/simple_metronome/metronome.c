/*!
 * a very simple application to use the flipper zero as metronome
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>
#include "notification/notification.h"
#include "notification/notification_messages.h"
#include "metronome_gui/menu.h"
#include "metronome_gui/metronome_view.h"

#define TAG __FILE__
#define DEFAULT_BPM 60

enum errorCodes {
    STATUS_SUCCESS = 0,
    STATUS_MALLOC_FAILED = -1,
};

typedef struct {
    bool active;
    bool currentState;
    uint8_t bpm;
    uint32_t maxCount;
    FuriTimer* timer;
    NotificationApp* notifications;
    metronomeMenu_t menu;
    MetroView_t view;
} metronomeCtx;

static uint8_t get_bpm(void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    return metronome->bpm;
}

static void set_bpm(uint8_t bpm, void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->bpm = bpm;
}

static void set_metronome_active(bool active, void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->active = active;
    if(metronome->active) {
        furi_timer_stop(metronome->timer);
        uint8_t const bpm_current = get_bpm(metronome);
        uint32_t const bpm_ms = (60u * 1000u) / bpm_current;
        uint32_t const bpm_ticks = furi_ms_to_ticks(bpm_ms);
        furi_timer_start(metronome->timer, bpm_ticks);
    } else {
        furi_timer_stop(metronome->timer);
    }
}

static void timer_callback(void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    FURI_LOG_I(TAG, "timer callback");
    notification_message(metronome->notifications, &sequence_blink_green_100);
    notification_message(metronome->notifications, &sequence_single_vibro);
    if(metronome->active) {
        static uint8_t bpm_last = DEFAULT_BPM;
        const uint8_t bpm_current = get_bpm(ctx);
        if(bpm_last != bpm_current) {
            furi_timer_stop(metronome->timer);
            uint32_t const bpm_ms = (60u * 1000u) / bpm_current;
            uint32_t const bpm_ticks = furi_ms_to_ticks(bpm_ms);
            bpm_last = bpm_current;
            furi_timer_start(metronome->timer, bpm_ticks);
        }
    }
    metronome->maxCount--;
}

static metronomeCtx* allocate_metronome(void) {
    FURI_LOG_I(TAG, "starting metronome");
    metronomeCtx* ctx = malloc(sizeof(metronomeCtx));
    if(ctx) {
        ctx->active = false;
        ctx->currentState = false;
        ctx->notifications = furi_record_open(RECORD_NOTIFICATION);
        ctx->timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, ctx);
        ctx->maxCount = 20u;
        ctx->bpm = DEFAULT_BPM;
        // ctx->menu = menu_alloc();
        ctx->view = view_alloc(get_bpm, set_bpm, set_metronome_active, ctx);
    }
    return ctx;
}

static void free_metronome(metronomeCtx* ctx) {
    FURI_LOG_I(TAG, "stopping metronome");
    view_free(ctx->view);
    // menu_free(ctx->menu);
    ctx->currentState = false;
    ctx->active = false;
    furi_timer_free(ctx->timer);
    furi_record_close(RECORD_NOTIFICATION);
    free(ctx);
}

static void metronome_display_main(metronomeCtx* metronome) {
    while(metronome->maxCount > 0) {
        // menu_run(metronome->menu);

        view_run(metronome->view);
        furi_delay_ms(100u);
    }

    furi_timer_stop(metronome->timer);
}

int32_t metronome_entry(void* p) {
    UNUSED(p);
    int32_t status = STATUS_SUCCESS;

    metronomeCtx* ctx = allocate_metronome();
    if(ctx == NULL) {
        FURI_LOG_W(TAG, "failed to malloc context");
        status = STATUS_MALLOC_FAILED;
    }

    if(status == STATUS_SUCCESS) {
        metronome_display_main(ctx);
    }

    free_metronome(ctx);

    return status;
}