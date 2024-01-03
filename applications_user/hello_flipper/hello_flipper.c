#include <stdbool.h>
#include <input/input.h>
#include <gui/canvas.h>
#include <gui/gui.h>
#include <gui/view_port.h>
#include <furi.h>

#define TAG "hello flipper"

static bool s_exitFlag = false;

static void input_callback(InputEvent* event, void* ctx) {
    UNUSED(ctx);
    FURI_LOG_I(TAG, "event callback %i called with key %i", event->type, event->key);
    if((event->key == InputKeyBack) && (event->type == InputTypeShort)) {
        s_exitFlag = true;
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    FURI_LOG_I(TAG, "draw callback called");
    char const* text = "hello world";
    const size_t middle_x = canvas_width(canvas) / 2u;
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, middle_x, 12, AlignCenter, AlignBottom, text);
}

int32_t hello_world(void* p) {
    UNUSED(p);
    FURI_LOG_I(TAG, "starting");
    ViewPort* view_port = view_port_alloc();
    if(!view_port) {
        FURI_LOG_W(TAG, "failed to allocate viewport");
        return -1;
    }
    FURI_LOG_I(TAG, "viewport allocated");

    view_port_draw_callback_set(view_port, draw_callback, NULL);
    view_port_input_callback_set(view_port, input_callback, NULL);
    FURI_LOG_I(TAG, "callbacks set");

    Gui* gui = furi_record_open(RECORD_GUI);
    if(!gui) {
        FURI_LOG_W(TAG, "failed to open gui");
        goto end;
    }
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    while(s_exitFlag == false) {
        view_port_update(view_port);
        furi_delay_ms(1000u);
        FURI_LOG_I(TAG, "mainloop");
    };
    FURI_LOG_I(TAG, "done");

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    furi_record_close(RECORD_GUI);
end:
    view_port_free(view_port);
    return 0;
}