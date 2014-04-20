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

#ifndef _GS_WPIXMAP_H_
#define _GS_WPIXMAP_H_

struct gs_widget_t * gs_wpixmap_create(struct gs_win_t *win,
        int x,int y, int width, int height);
void gs_wpixmap_destroy(struct gs_widget_t *wd);
void gs_wpixmap_set_image(struct gs_widget_t *wd, union gs_pixel_t *image);
void gs_wpixmap_set_alpha_on_color(struct gs_widget_t *wd, int color, uint8 alpha);
void gs_wpixmap_set_alpha(struct gs_widget_t *wd, uint8 alpha);
void gs_wpixmap_draw(struct gs_widget_t *wd);
void gs_wpixmap_draw2(struct gs_widget_t *wd, int color);
void gs_wpixmap_clear(struct gs_widget_t *wd);

#endif /* !_GS_WPIXMAP_H */

