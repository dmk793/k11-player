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
#include <debug.h>
#include "gs.h"
#include "gsp.h"

/*
 * generic widget destructor
 */
void gs_widget_destroy(struct gs_widget_t *wd)
{
    gs_win_widget_remove(wd->pwin, wd);
    gsp_free(wd);
}

#ifdef GS_SINGLE_WINDOW
/*
 *
 */
void gs_widget_refresh(struct gs_widget_t *wd)
{
    if (!wd)
        return;

    gsp_mutex_lock();
    wd->map = 1;
    gsp_mutex_unlock();
}
#endif

