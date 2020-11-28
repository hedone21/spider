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
#include "server/event.h"
#include "server/server.h"
#include "server/spider_assert.h"
#include "common/log.h"

struct spider_wlroots_server *server;

static bool spider_backend_wlroots_init(struct spider_server *server, void *data) {
    spider_log("wlroots init\n");

    server = calloc(1, sizeof(*server));
    spider_assert(server != NULL);

    server->wl_display = wl_display_create();
    spider_assert(server->wl_display != NULL):
    server->backend = wlr_backend_autocreate(server->wl_display, NULL);
    spider_assert(server->backend != NULL):
	server->noop_backend = wlr_noop_backend_create(server->wl_display);

	server->renderer = wlr_backend_get_renderer(server->backend);
	wlr_renderer_init_wl_display(server->renderer, server->wl_display);

	server->compositor = wlr_compositor_create(server->wl_display, server->renderer);
	wlr_data_device_manager_create(server->wl_display);

	server->output_layout = wlr_output_layout_create();
	server->new_output.notify = handle_new_output;
	wl_signal_add(&server->backend->events.new_output, &server->new_output);

	server->xdg_shell = wlr_xdg_shell_create(server->wl_display);
	server->new_xdg_surface.notify = handle_new_xdg_surface;
	wl_signal_add(&server->xdg_shell->events.new_surface,
			&server->new_xdg_surface);

	server->layer_shell = wlr_layer_shell_v1_create(server->wl_display);
	server->layer_shell_surface.notify = handle_layer_shell_surface;
	wl_signal_add(&server->layer_shell->events.new_surface,
			&server->layer_shell_surface);

	server->cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(server->cursor, server->output_layout);
	server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
	wlr_xcursor_manager_load(server->cursor_mgr, 1);

    // create_cursor()

	server->new_input.notify = handle_new_input;
	wl_signal_add(&server->backend->events.new_input, &server->new_input);

	server->seat = wlr_seat_create(server->wl_display, "seat0");
	server->request_cursor.notify = handle_seat_request_cursor;
	wl_signal_add(&server->seat->events.request_set_cursor,
			&server->request_cursor);

	/* TODO custom interface */
	const char *socket = wl_display_add_socket_auto(server->wl_display);
	if (!socket) {
        goto out;
	}

	if (!wlr_backend_start(server->backend)) {
        goto out;
	}

	setenv("WAYLAND_DISPLAY", socket, true);

	server->wl_event_loop = wl_display_get_event_loop(server->wl_display);

    // TODO launch client
	spider_log("Running Wayland compositor on WAYLAND_DISPLAY=%s\n", socket);
	wl_display_run(compositor->wl_display);

out:
	wl_display_destroy_clients(compositor->wl_display);
	wl_display_destroy(compositor->wl_display);

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
