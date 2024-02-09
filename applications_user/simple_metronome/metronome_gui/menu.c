
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

struct metronomeMenu {
    bool active;
    NotificationType notif_type;
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    VariableItemList* var_item_list;
};

static void notification_type_changed(VariableItem* item) {
    uint8_t index = variable_item_get_current_value_index(item);
    variable_item_set_current_value_text(item, notification_type[index]);
    /* TODO: Forward setting to metronome */
}

static uint32_t menu_exit(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

metronomeMenu_t menu_alloc(void) {
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
    VariableItem* item = variable_item_list_add(
        menu->var_item_list,
        "notific. type",
        COUNT_OF(notification_type),
        notification_type_changed,
        menu);

    view_set_previous_callback(variable_item_list_get_view(menu->var_item_list), menu_exit);

    variable_item_set_current_value_index(item, menu->notif_type);
    view_dispatcher_add_view(
        menu->view_dispatcher, CURRENT_VIEW, variable_item_list_get_view(menu->var_item_list));
    view_dispatcher_switch_to_view(menu->view_dispatcher, CURRENT_VIEW);
    return menu;
}

void menu_run(metronomeMenu_t menu) {
    if(!menu) return;
    view_dispatcher_run(menu->view_dispatcher);
}

void menu_free(metronomeMenu_t menu) {
    if(menu) {
        view_dispatcher_remove_view(menu->view_dispatcher, CURRENT_VIEW);
        variable_item_list_free(menu->var_item_list);
        view_dispatcher_free(menu->view_dispatcher);
        furi_record_close(RECORD_GUI);
        free(menu);
    }
}