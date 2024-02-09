
#include "menu.h"
#include <stdbool.h>

#include <furi.h>

#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/variable_item_list.h>

#include "../metronome.h"

#define TAG __FILE__

#define CURRENT_VIEW 0u

const char* const notification_type[NOTIF_MAX] = {
    [NOTIF_SOUND] = "sound",
    [NOTIF_BUZZER] = "buzzer",
    [NOTIF_LED] = "led",
};

const char* const on_off_option_type[2u] = {"OFF", "ON"};

static bool s_back = false;

struct metronomeMenu {
    bool active;
    NotificationType notif_type;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* var_item_list;
    ScreenRequestCb screen_req;
    NotificationTypeCb notif_cb;
    void* cb_ctx;
};

static void notification_type_changed(VariableItem* item) {
    metronomeMenu_t menu = (metronomeMenu_t)variable_item_get_context(item);
    uint8_t const notificationIndex =
        variable_item_list_get_selected_item_index(menu->var_item_list);
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, on_off_option_type[index]);
    menu->notif_cb(index > 0, (NotificationType)notificationIndex, menu->cb_ctx);
}

static uint32_t menu_exit(void* context) {
    UNUSED(context);
    s_back = true;
    FURI_LOG_I(TAG, "menu exit");
    return VIEW_NONE;
}

metronomeMenu_t menu_alloc(
    ScreenRequestCb screen_req,
    NotificationTypeCb notification_type_cb,
    NotificationGetCb notification_get_cb,
    void* cb_ctx) {
    struct metronomeMenu* const menu = malloc(sizeof(struct metronomeMenu));
    furi_assert(menu != NULL);

    menu->gui = furi_record_open(RECORD_GUI);
    furi_assert(menu->gui);

    menu->notif_type = NOTIF_SOUND; /* TODO: get metronome settings from metronome */

    menu->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(menu->view_dispatcher);
    view_dispatcher_set_event_callback_context(menu->view_dispatcher, menu);
    view_dispatcher_attach_to_gui(menu->view_dispatcher, menu->gui, ViewDispatcherTypeFullscreen);

    menu->var_item_list = variable_item_list_alloc();
    furi_assert(notification_get_cb != NULL);
    for(size_t index = 0u; index < (size_t)NOTIF_MAX; ++index) {
        NotificationType notification = (NotificationType)index;
        VariableItem* item = variable_item_list_add(
            menu->var_item_list,
            notification_type[notification],
            COUNT_OF(on_off_option_type),
            notification_type_changed,
            menu);
        /* set current settings */
        bool active = notification_get_cb(notification, cb_ctx);
        uint8_t active_index = active ? 1u : 0u;
        variable_item_set_current_value_index(item, active_index);
        variable_item_set_current_value_text(item, on_off_option_type[active_index]);
    }

    view_set_previous_callback(variable_item_list_get_view(menu->var_item_list), menu_exit);

    view_dispatcher_add_view(
        menu->view_dispatcher, CURRENT_VIEW, variable_item_list_get_view(menu->var_item_list));
    view_dispatcher_switch_to_view(menu->view_dispatcher, CURRENT_VIEW);
    menu->screen_req = screen_req;
    menu->notif_cb = notification_type_cb;
    menu->cb_ctx = cb_ctx;
    return menu;
}

void menu_run(metronomeMenu_t menu) {
    if(!menu) return;
    if(s_back) {
        FURI_LOG_I(TAG, "menu exit active");
        s_back = false;
        menu->screen_req(menu->cb_ctx);
        view_dispatcher_stop(menu->view_dispatcher);
    } else {
        view_dispatcher_run(menu->view_dispatcher);
    }
}

void menu_free(metronomeMenu_t menu) {
    FURI_LOG_I(TAG, "menu free active");
    if(menu) {
        view_dispatcher_remove_view(menu->view_dispatcher, CURRENT_VIEW);
        variable_item_list_free(menu->var_item_list);
        view_dispatcher_free(menu->view_dispatcher);
        furi_record_close(RECORD_GUI);
        free(menu);
    }
}