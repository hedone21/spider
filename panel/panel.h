/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __PANEL_PANEL_H__
#define __PANEL_PANEL_H__

#include <wayland-client.h>
#include <gtk/gtk.h>
#include <gdk/gdkwayland.h>
#include <wlr/types/wlr_layer_shell_v1.h>
#include "protocol/spider-client-protocol.h"
#include "protocol/xdg-shell-client-protocol.h"

struct spider_panel {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_surface *surface;
	struct wl_shm *shm;
	struct wl_seat *seat;
	struct wl_output *output;
	struct wlr_layer_shell *layer_shell;

	struct xdg_wm_base *wm_base;
	struct xdg_shell *xdg_shell;
	struct xdg_surface *xdg_surface;

	GdkDisplay *gdk_display;

	int panel_width;
	int panel_height;
	// struct spider_panel_manager_v1 *panel_manager;
	//
	GtkWidget *window;
	GtkWidget *panel;
};

int panel_init(struct spider_panel *panel);

#endif
