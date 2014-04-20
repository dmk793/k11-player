/* 
 *     This file is part of K11, hardware multimedia player.
 * 
 * Copyright (C) 2014 Dmitry Kobylin
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef _GSP_H_
#define _GSP_H_

#include <types.h>
#include "gs.h"

struct gs_private_t {
    int initialized;

    struct gs_win_t *wmhead;
#define GSP_MUTEX_1   (1 << 0)
    BASE_TYPE mutex;
#define GSP_EVENT_MASK_VSYNC    (1 << 0)
    BASE_TYPE event;
    int abuf; /* active buffer */
};

extern struct gs_private_t gsp;

void gsp_hw_init();
void gsp_wait_vsync();
void gsp_win_map(struct gs_win_t *win, union gs_pixel_t *dst);
void gsp_win_map2(struct gs_win_t *win, union gs_pixel_t *dst, struct gs_pane_t *pane);
void gsp_image_map(struct gs_win_t *win, int x, int y, uint8 *idata, int iwidth, int iheight);
void gsp_win_refresh(struct gs_win_t *win);
#ifndef GS_SINGLE_WINDOW
void gsp_win_refresh_finish(struct gs_win_t *win);
#else
void gsp_win_refresh_widgets(struct gs_win_t *win);
#endif
void gsp_set_active_buffer();
void* gsp_malloc(int size);
void gsp_free(void *p);
void gsp_memcpy(void *dst, void *src, int size);
void gsp_memset(void *mem, int pat, int size);
void gsp_mutex_lock();
void gsp_mutex_unlock();
void gsp_enter_crit();
void gsp_exit_crit();
void gsp_init_end();
int gsp_initialized();

#endif /* !_GSP_H_ */

