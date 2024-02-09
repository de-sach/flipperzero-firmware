#pragma once

typedef struct metronomeMenu* metronomeMenu_t;

metronomeMenu_t menu_alloc(void);
void menu_run(metronomeMenu_t menu);
void menu_free(metronomeMenu_t menu);