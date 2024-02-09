#include "metronome_view.h"

#include "gui/gui.h"
#include "gui/canvas.h"
#include "input/input.h"
#include "furi.h"

#include <string.h>

#define TAG "MV"
/* 
   idea: BPM central, up down speed up/slow down (long vs short press)
   right: enter settings
   left: switch notification modes
   center: start stop
   back: exit
*/
struct MetroView {
    void* bpm_cb_ctx;
    GetBpmCb get_bpm;
    SetBpmCb set_bpm;
    Gui* gui;
    ViewPort* bpm_vp;
    bool active;
    SetActiveCb set_active;
    SetEnabledCb set_enabled;
    ScreenRequestCb screen_req;
};

static void input_bpm_cb(InputEvent* event, void* ctx) {
    MetroView_t view = (MetroView_t)ctx;
    uint8_t stepSize = 0u;
    switch(event->type) {
    case InputTypeShort:
        stepSize = 1u;
        break;
    case InputTypeLong:
    case InputTypeRepeat:
        stepSize = 5u;
        break;
    default:
        /* don't handle*/
        break;
    }
    uint8_t bpm = view->get_bpm(view->bpm_cb_ctx);

    switch(event->key) {
    case InputKeyUp:
        bpm = (bpm < (UINT8_MAX - stepSize)) ? (uint8_t)(bpm + stepSize) : UINT8_MAX;
        break;
    case InputKeyDown:
        bpm = (bpm > stepSize) ? (uint8_t)(bpm - stepSize) : 0u;
        break;
    case InputKeyOk:
        view->active = !view->active;
        view->set_active(view->active, view->bpm_cb_ctx);
        break;
    case InputKeyBack:
        view->set_enabled(false, view->bpm_cb_ctx);
        break;
    case InputKeyRight:
        view->screen_req(view->bpm_cb_ctx);
        /* open menu */ break;
    default:
        /*nothing*/ break;
    }
    if(stepSize > 0u) {
        view->set_bpm(bpm, view->bpm_cb_ctx);
        view_port_update(view->bpm_vp);
    }
}

static void draw_bpm_cb(Canvas* canvas, void* ctx) {
    MetroView_t view = (MetroView_t)ctx;
    char bpmBuf[4u] = {0}; /* BPM: XXX\0 */
    uint8_t bpm = view->get_bpm(view->bpm_cb_ctx);
    int32_t result = snprintf(bpmBuf, sizeof(bpmBuf), "%d", bpm);
    furi_assert((result > 0) && (result < (int32_t)sizeof(bpmBuf)));
    canvas_set_font(canvas, FontBigNumbers);

    FURI_LOG_I(TAG, "bpm buffer: %s", bpmBuf);
    size_t center_x = canvas_width(canvas) / 2;
    size_t const y_pos = (canvas_height(canvas) / 2) + 5;

    canvas_draw_str_aligned(canvas, center_x + 20, y_pos, AlignLeft, AlignCenter, bpmBuf);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, center_x - 20, y_pos, AlignLeft, AlignCenter, "BPM: ");
}

MetroView_t view_alloc(
    GetBpmCb get_bpm,
    SetBpmCb set_bpm,
    SetActiveCb set_active,
    SetEnabledCb set_enabled,
    ScreenRequestCb screen_req,
    void* ctx) {
    struct MetroView* view = malloc(sizeof(struct MetroView));
    furi_assert(view);
    view->gui = furi_record_open(RECORD_GUI);
    view->bpm_vp = view_port_alloc();
    view->bpm_cb_ctx = ctx;
    view->get_bpm = get_bpm;
    view->set_bpm = set_bpm;
    view->set_active = set_active;
    view->set_enabled = set_enabled;
    view->screen_req = screen_req;
    view_port_draw_callback_set(view->bpm_vp, draw_bpm_cb, view);
    view_port_input_callback_set(view->bpm_vp, input_bpm_cb, view);
    gui_add_view_port(view->gui, view->bpm_vp, GuiLayerFullscreen);
    view_port_update(view->bpm_vp);
    return view;
}

void view_run(MetroView_t view) {
    furi_assert(view);
}

void view_free(MetroView_t view) {
    furi_assert(view);

    gui_remove_view_port(view->gui, view->bpm_vp);
    view_port_free(view->bpm_vp);
    furi_record_close(RECORD_GUI);
    free(view);
}