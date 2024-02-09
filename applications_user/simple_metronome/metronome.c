/*!
 * a very simple application to use the flipper zero as metronome
 * 
 */

#include <stdint.h>
#include <stdbool.h>
#include <furi.h>
#include "notification/notification.h"
#include "notification/notification_messages.h"

#define TAG __FILE__
#define DEFAULT_BPM 60

enum errorCodes {
    STATUS_SUCCESS = 0,
    STATUS_MALLOC_FAILED = -1,
};

typedef struct {
    bool active;
    bool currentState;
    uint32_t maxCount;
    FuriTimer* timer;
    NotificationApp* notifications;
} metronomeCtx;

static void timer_callback(void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    FURI_LOG_I(TAG, "timer callback");
    notification_message(metronome->notifications, &sequence_blink_green_100);
    notification_message(metronome->notifications, &sequence_single_vibro);
    metronome->maxCount--;
}

static metronomeCtx* allocate_metronome(void) {
    FURI_LOG_I(TAG, "starting metronome");
    metronomeCtx* ctx = malloc(sizeof(metronomeCtx));
    if(ctx) {
        ctx->active = true;
        ctx->currentState = false;
        ctx->notifications = furi_record_open(RECORD_NOTIFICATION);
        ctx->timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, ctx);
        ctx->maxCount = 10u;
    }
    return ctx;
}

static void free_metronome(metronomeCtx* ctx) {
    FURI_LOG_I(TAG, "stopping metronome");
    ctx->currentState = false;
    ctx->active = false;
    furi_timer_free(ctx->timer);
    furi_record_close(RECORD_NOTIFICATION);
    free(ctx);
}

static void metronome_display_main(metronomeCtx* metronome) {
    uint32_t const bpmInMs = (60u * 1000u) / DEFAULT_BPM;
    uint32_t const bpmInTicks = furi_ms_to_ticks(bpmInMs);
    furi_timer_start(metronome->timer, bpmInTicks);

    while(metronome->active) {
        furi_delay_ms(100u);
        if(metronome->maxCount == 0u) {
            metronome->active = false;
        }
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