#pragma once

typedef struct metronomeMenu* metronomeMenu_t;
typedef void (*ScreenRequestCb)(void* ctx);

metronomeMenu_t menu_alloc(ScreenRequestCb screen_req, void* cb_ctx);
void menu_run(metronomeMenu_t menu);
void menu_free(metronomeMenu_t menu);