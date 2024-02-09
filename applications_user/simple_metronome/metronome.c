/*!
 * a very simple application to use the flipper zero as metronome
 * 
 */

#include <stdint.h>
#include <stdbool.h>

#include <furi.h>

#include "notification/notification.h"
#include "notification/notification_messages.h"
#include "notification/notification_messages_notes.h"
#include "storage/storage.h"

#include "metronome_gui/menu.h"
#include "metronome_gui/metronome_view.h"
#include "metronome.h"

#define TAG __FILE__
#define DEFAULT_BPM 60
#define SETTINGS_FILE "metronome_settings.txt"

enum errorCodes {
    STATUS_SUCCESS = 0,
    STATUS_MALLOC_FAILED = -1,
};

typedef enum screen {
    E_SCREEN_MAIN,
    E_SCREEN_MENU,
} screen_t;

typedef struct {
    bool active;
    bool currentState;
    uint8_t bpm;
    bool exit;
    FuriTimer* timer;
    NotificationApp* notifications;
    metronomeMenu_t menu;
    MetroView_t view;
    screen_t screen;
    bool notification_type_settings[NOTIF_MAX];
    RythmType rythm;
    uint8_t current_beat;
} metronomeCtx;

const uint8_t rythmEmphasis[RYTHM_MAX] = {
    [RYTHM_NONE] = 0,
    [RYTHM_2_4] = 2,
    [RYTHM_3_4] = 3,
    [RYTHM_4_4] = 4,
    [RYTHM_6_8] = 6,
};

/* required to be static */
static const NotificationSequence sequence_play_sound = {
    &message_note_a4,
    &message_delay_50,
    &message_sound_off,
    NULL, /* null at end of sequence required */
};

static const NotificationSequence sequence_play_sound_em = {
    &message_note_e5,
    &message_delay_50,
    &message_sound_off,
    NULL, /* null at end of sequence required */
};

const NotificationSequence sequence_vibro_short = {
    &message_vibro_on,
    &message_delay_50,
    &message_vibro_off,
    NULL,
};

const NotificationSequence sequence_vibro_short_em = {
    &message_vibro_on,
    &message_delay_100,
    &message_vibro_off,
    NULL,
};

static void rythm_callback(RythmType rythm, void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->rythm = rythm;
}

static bool get_notification_callback(NotificationType notification, void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    return metronome->notification_type_settings[notification];
}

static void set_notification_callback(bool active, NotificationType notification, void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->notification_type_settings[notification] = active;
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);
    bool open_success =
        storage_file_open(file, APP_DATA_PATH(SETTINGS_FILE), FSAM_WRITE, FSOM_OPEN_ALWAYS);
    if(open_success) {
        size_t bytes_written =
            storage_file_write(file, metronome->notification_type_settings, (size_t)NOTIF_MAX);
        furi_assert(bytes_written == (size_t)NOTIF_MAX);
        storage_file_close(file);
    }
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
}

static uint8_t get_bpm(void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    return metronome->bpm;
}

static void set_bpm(uint8_t bpm, void* ctx) {
    FURI_LOG_I(TAG, "set bpm callback %u", bpm);
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->bpm = bpm;
}

static void set_metronome_active(bool active, void* ctx) {
    FURI_LOG_I(TAG, "metronome active cb:  %s", active ? "on" : "off");
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->active = active;
    metronome->current_beat = rythmEmphasis[metronome->rythm];
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

static void set_enabled(bool enabled, void* ctx) {
    FURI_LOG_I(TAG, "set enabled cb");
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->exit = !enabled;
    set_metronome_active(false, ctx);
}

static void menu_screen_req(void* ctx) {
    FURI_LOG_I(TAG, "set menu screen req");
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->screen = E_SCREEN_MENU;
}

static void main_screen_req(void* ctx) {
    FURI_LOG_I(TAG, "set main screen req");
    metronomeCtx* metronome = (metronomeCtx*)ctx;
    metronome->screen = E_SCREEN_MAIN;
}

static void timer_callback(void* ctx) {
    metronomeCtx* metronome = (metronomeCtx*)ctx;

    metronome->current_beat++;
    if(metronome->current_beat >= rythmEmphasis[metronome->rythm]) {
        metronome->current_beat = 0u;
    }

    if(metronome->current_beat > 0) {
        if(metronome->notification_type_settings[NOTIF_SOUND]) {
            notification_message(metronome->notifications, &sequence_play_sound);
        }
        if(metronome->notification_type_settings[NOTIF_LED]) {
            notification_message(metronome->notifications, &sequence_blink_green_100);
        }
        if(metronome->notification_type_settings[NOTIF_BUZZER]) {
            notification_message(metronome->notifications, &sequence_vibro_short);
        }
    } else {
        if(metronome->notification_type_settings[NOTIF_SOUND]) {
            notification_message(metronome->notifications, &sequence_play_sound_em);
        }
        if(metronome->notification_type_settings[NOTIF_LED]) {
            notification_message(metronome->notifications, &sequence_blink_red_100);
        }
        if(metronome->notification_type_settings[NOTIF_BUZZER]) {
            notification_message(metronome->notifications, &sequence_vibro_short_em);
        }
    }
    FURI_LOG_I(TAG, "notifications done");
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
}

static metronomeCtx* allocate_metronome(void) {
    FURI_LOG_I(TAG, "starting metronome");
    metronomeCtx* ctx = malloc(sizeof(metronomeCtx));
    if(ctx) {
        ctx->active = false;
        ctx->currentState = false;
        ctx->exit = false;
        ctx->notifications = furi_record_open(RECORD_NOTIFICATION);
        ctx->timer = furi_timer_alloc(timer_callback, FuriTimerTypePeriodic, ctx);
        ctx->bpm = DEFAULT_BPM;
        ctx->menu = NULL;
        ctx->view =
            view_alloc(get_bpm, set_bpm, set_metronome_active, set_enabled, menu_screen_req, ctx);
        ctx->rythm = RYTHM_NONE;
        Storage* storage = furi_record_open(RECORD_STORAGE);
        File* file = storage_file_alloc(storage);

        bool open_success = storage_file_open(
            file, APP_DATA_PATH(SETTINGS_FILE), FSAM_READ_WRITE, FSOM_OPEN_ALWAYS);
        if(open_success) {
            bool buffer[NOTIF_MAX] = {false, false, false};
            size_t const bytes_read = storage_file_read(file, buffer, sizeof(buffer));
            if(bytes_read == NOTIF_MAX) {
                for(size_t index = 0u; index < (size_t)NOTIF_MAX; index++) {
                    ctx->notification_type_settings[index] = buffer[index];
                }
            } else {
                for(size_t index = 0u; index < (size_t)NOTIF_MAX; index++) {
                    ctx->notification_type_settings[index] = true;
                    buffer[index] = true;
                }
                size_t const bytes_written = storage_file_write(file, buffer, sizeof(buffer));
                furi_assert(bytes_written == ((size_t)NOTIF_MAX));
            }
            storage_file_close(file);
        } else {
            for(size_t index = 0u; index < (size_t)NOTIF_MAX; index++) {
                ctx->notification_type_settings[index] = true;
            }
        }
        storage_file_free(file);
        furi_record_close(RECORD_STORAGE);
    }
    return ctx;
}

static void free_metronome(metronomeCtx* ctx) {
    FURI_LOG_I(TAG, "stopping metronome");
    if(ctx->view != NULL) {
        view_free(ctx->view);
    }
    if(ctx->menu != NULL) {
        menu_free(ctx->menu);
    }
    ctx->currentState = false;
    ctx->active = false;
    ctx->exit = true;
    furi_timer_free(ctx->timer);
    furi_record_close(RECORD_NOTIFICATION);
    free(ctx);
}

static void metronome_display_main(metronomeCtx* metronome) {
    while(metronome->exit == false) {
        static screen_t s_prev = E_SCREEN_MENU;
        if(metronome->screen != s_prev) {
            FURI_LOG_I(TAG, "changing screen");
        }
        switch(metronome->screen) {
        case E_SCREEN_MAIN:
            if(s_prev == E_SCREEN_MENU) {
                menu_free(metronome->menu);
                metronome->menu = NULL;
            }
            view_run(metronome->view);
            break;
        case E_SCREEN_MENU:
            if(s_prev == E_SCREEN_MAIN) {
                metronome->menu = menu_alloc(
                    main_screen_req,
                    set_notification_callback,
                    get_notification_callback,
                    rythm_callback,
                    metronome);
            }
            menu_run(metronome->menu);
            break;
        default: /*should not happen*/
            furi_assert(false);
            break;
        }
        s_prev = metronome->screen;
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