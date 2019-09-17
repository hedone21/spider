/*
 * Copyright (c) 2014 Jari Vetoniemi
 * Copyright (c) 2017, 2018 Drew DeVault
 * Copyright (c) 2019 Minyoung.Go <hedone21@gmail.com>
 * 
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

#ifndef __SPIDER_DESKTOP_H__
#define __SPIDER_DESKTOP_H__

#include <wayland-server.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_layer_shell_v1.h>
/* TODO: Unstable wayland interfaces
#include <wlr/types/wlr_xdg_output_v6.h>
*/
#include <wlr/util/log.h>
#include <wlr/backend.h>
#include <xkbcommon/xkbcommon.h>
#include <stdbool.h>
#include "spider/output.h"
#include "spider/layer.h"

struct spider_options {
	char *panel;
	char *shell;
	char *server;
	bool debug;
	bool verbose;
};

extern struct spider_options g_options;

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
struct render_data {
	struct wlr_output *output;
	struct wlr_renderer *renderer;
	struct spider_view *view;
	struct timespec *when;
};

/* For brevity's sake, struct members are annotated where they are used. */
enum spider_cursor_mode {
	SPIDER_CURSOR_PASSTHROUGH,
	SPIDER_CURSOR_MOVE,
	SPIDER_CURSOR_RESIZE,
};

struct spider_desktop {
	struct wl_display *wl_display;
	struct wl_event_loop *wl_event_loop;
	struct wlr_compositor *compositor;
	struct wlr_backend *backend;
	struct wlr_backend *noop_backend;
	struct wlr_renderer *renderer;

	struct wlr_xdg_shell *xdg_shell;
	struct wl_listener new_xdg_surface;
	struct wl_list views;

	struct wlr_cursor *cursor;
	struct wlr_xcursor_manager *cursor_mgr;
	struct wl_listener cursor_motion;
	struct wl_listener cursor_motion_absolute;
	struct wl_listener cursor_button;
	struct wl_listener cursor_axis;
	struct wl_listener cursor_frame;

	struct wlr_seat *seat;
	struct wl_listener new_input;
	struct wl_listener request_cursor;
	struct wl_list keyboards;
	enum spider_cursor_mode cursor_mode;
	struct spider_view *grabbed_view;
	double grab_x, grab_y;
	int grab_width, grab_height;
	uint32_t resize_edges;

	struct wlr_output_layout *output_layout;
	struct wl_list outputs;
	struct wl_listener new_output;

	int client_server_pid;
	int client_shell_pid;

	/* Unstable Interface */
	struct wlr_layer_shell_v1 *layer_shell;
	struct wl_listener layer_shell_surface;
	struct wlr_xdg_shell_v6 *xdg_shell_v6;
	struct wl_listener xdg_shell_v6_surface;
};

struct spider_keyboard {
	struct wl_list link;
	struct spider_desktop *desktop;
	struct wlr_input_device *device;

	struct wl_listener modifiers;
	struct wl_listener key;
};

int preinit_desktop();
int init_desktop();

#endif /* __SERVER_H__ */
