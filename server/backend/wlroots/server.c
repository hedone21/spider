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

#include <unistd.h>
#include "server.h"
#include "output.h"
#include "server/event.h"
#include "server/server.h"
#include "server/spider_assert.h"
#include "common/log.h"

struct spider_wlroots_server *wlserver;

static bool spider_backend_wlroots_init(struct spider_server *server, void *data) {
    spider_log("wlroots init\n");

    wlserver = calloc(1, sizeof(*wlserver));
    spider_assert(wlserver != NULL);

    wlserver->wl_display = wl_display_create();
    spider_assert(wlserver->wl_display != NULL);
    wlserver->backend = wlr_backend_autocreate(wlserver->wl_display, NULL);
    spider_assert(wlserver->backend != NULL);
	wlserver->noop_backend = wlr_noop_backend_create(wlserver->wl_display);

	wlserver->renderer = wlr_backend_get_renderer(wlserver->backend);
	wlr_renderer_init_wl_display(wlserver->renderer, wlserver->wl_display);

	wlserver->compositor = wlr_compositor_create(wlserver->wl_display, wlserver->renderer);
	wlr_data_device_manager_create(wlserver->wl_display);

	wlserver->output_layout = wlr_output_layout_create();
	wlserver->new_output.notify = spider_wlroots_output_handle_new;
	wl_signal_add(&wlserver->backend->events.new_output, &wlserver->new_output);

	wlserver->xdg_shell = wlr_xdg_shell_create(wlserver->wl_display);
	// wlserver->new_xdg_surface.notify = handle_new_xdg_surface;
	wlserver->new_xdg_surface.notify = NULL;
	wl_signal_add(&wlserver->xdg_shell->events.new_surface,
			&wlserver->new_xdg_surface);

	wlserver->cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(wlserver->cursor, wlserver->output_layout);
	wlserver->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
	wlr_xcursor_manager_load(wlserver->cursor_mgr, 1);

    // create_cursor()

	// wlserver->new_input.notify = handle_new_input;
	wlserver->new_input.notify = NULL;
	wl_signal_add(&wlserver->backend->events.new_input, &wlserver->new_input);

	wlserver->seat = wlr_seat_create(wlserver->wl_display, "seat0");
	// wlserver->request_cursor.notify = handle_seat_request_cursor;
	wlserver->request_cursor.notify = NULL;
	wl_signal_add(&wlserver->seat->events.request_set_cursor,
			&wlserver->request_cursor);

	/* TODO custom interface */
	const char *socket = wl_display_add_socket_auto(wlserver->wl_display);
	if (!socket) {
        goto out;
	}

	if (!wlr_backend_start(wlserver->backend)) {
        goto out;
	}

	setenv("WAYLAND_DISPLAY", socket, true);

	wlserver->wl_event_loop = wl_display_get_event_loop(wlserver->wl_display);

    // TODO launch client
	spider_log("Running Wayland compositor on WAYLAND_DISPLAY=%s\n", socket);
	wl_display_run(wlserver->wl_display);

out:
	wl_display_destroy_clients(wlserver->wl_display);
	wl_display_destroy(wlserver->wl_display);

	return 0;
}

static void spider_backend_wlroots_run(struct spider_server *server, void *data) {
    spider_log("wlroots run\n");
}

static void spider_backend_wlroots_free(struct spider_server *server, void *data) {
    spider_log("wlroots free\n");
}

extern struct spider_backend_server spider_backend_server = {
    .init = spider_backend_wlroots_init,
    .run = spider_backend_wlroots_run,
    .free = spider_backend_wlroots_free,
    .data = NULL,
};
