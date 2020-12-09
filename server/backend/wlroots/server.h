/*
 * Copyright (c) 2020, 2021 Minyoung.Go <hedone21@gmail.com>
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

#ifndef SPIDER_BACKEND_WLROOTS_SERVER_H
#define SPIDER_BACKEND_WLROOTS_SERVER_H

#include "wlroots.h"
#include "output_mngr.h"
#include "server/backend/server.h"

struct spider_wlroots_server {
	struct wl_display *wl_display;
	struct wl_event_loop *wl_event_loop;
	struct wlr_compositor *compositor;
	struct wlr_backend *backend;
	struct wlr_backend *noop_backend;
	struct wlr_renderer *renderer;

	struct wlr_xdg_shell *xdg_shell;
	struct wl_listener new_xdg_surface;

	struct wlr_cursor *cursor;
	struct wlr_xcursor_manager *cursor_mgr;

	struct wlr_seat *seat;
	struct wl_listener new_input;
	struct wl_listener request_cursor;

	/* Unstable Interface */
	struct wlr_layer_shell_v1 *layer_shell;
	struct wl_listener layer_shell_surface;
	struct wlr_xdg_shell_v6 *xdg_shell_v6;
	struct wl_listener xdg_shell_v6_surface;

    /* Custom Structure */
    struct spider_wlroots_output_mngr *output_mngr;
};

/* This is external use */
struct spider_backend_server spider_backend_server;

#endif
