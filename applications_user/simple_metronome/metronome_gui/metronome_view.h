#pragma once

typedef struct MetroView* MetroView_t;

MetroView_t view_alloc(void);
void view_run(MetroView_t view);
void view_free(MetroView_t view);