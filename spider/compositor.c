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

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "spider/compositor.h"
#include "spider/cursor.h"
#include "spider/input.h"
#include "spider/launcher.h"
#include "spider/layer.h"
#include "spider/seat.h"
#include "spider/view.h"
#include "spider/xdg_shell.h"
#include "common/global_vars.h"
#include "common/log.h"
#include "common/util.h"
#include "protocol/spider-compositor-manager-v1-protocol.h"

static struct spider_compositor *compositor;

int preinit_compositor()
{
	compositor = calloc(1, sizeof(*compositor));
	if (compositor == NULL) {
		spider_err("Allocation Failed\n");
		return -1;
	}
	/* The Wayland display is managed by libwayland. It handles accepting
	 * clients from the Unix socket, manging Wayland globals, and so on. */
	compositor->wl_display = wl_display_create();
	/* The backend is a wlroots feature which abstracts the underlying input and
	 * output hardware. The autocreate option will choose the most suitable
	 * backend based on the current environment, such as opening an X11 window
	 * if an X11 compositor is running. The NULL argument here optionally allows you
	 * to pass in a custom renderer if wlr_renderer doesn't meet your needs. The
	 * backend uses the renderer, for example, to fall back to software cursors
	 * if the backend does not support hardware cursors (some older GPUs
	 * don't). */
	compositor->backend = wlr_backend_autocreate(compositor->wl_display, NULL);
	compositor->noop_backend = wlr_noop_backend_create(compositor->wl_display);

	if (!compositor->backend) {
		spider_err("Unable to create backend");
		return -1;
	}

	return 0;
}

static void set_background(struct wl_client *client,
		struct wl_resource *resource,
		struct wl_resource *surface)
{
	struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface);

	struct spider_view *view;
	spider_list_for_each(view, &compositor->views, link) {
		if (view->xdg_surface->surface == wlr_surface) {
			spider_dbg("set background %p / %p\n", view, wlr_surface);
			view->layer = LAYER_BACKGROUND;
		}
	}
}

static void set_bar(struct wl_client *client,
		struct wl_resource *resource,
		struct wl_resource *surface,
		uint32_t bar_type, uint32_t position)
{
	struct wlr_surface *wlr_surface = wlr_surface_from_resource(surface);
	const char *type;
	int layer;

	switch (bar_type) {
	case 0:
		type = "status-bar";
		layer = LAYER_STATUS_BAR;
		break;
	case 1:
		type = "navigation-bar";
		layer = LAYER_STATUS_BAR;
		break;
	default:
		break;
	}

	struct spider_view *view;
	spider_list_for_each(view, &compositor->views, link) {
		if (view->xdg_surface->surface == wlr_surface) {
			spider_dbg("set %s %p / %p\n", type, view, wlr_surface);
			view->layer = LAYER_STATUS_BAR;
		}
	}
}

static const struct spider_compositor_manager_v1_interface spider_compositor_implementation = {
	.set_background = set_background,
	.set_bar = set_bar,
};

static void bind_spider_compositor(struct wl_client *client,
		   void *data, uint32_t version, uint32_t id)
{
	struct spider_compositor *compositor = data;
	struct wl_resource *resource;
	spider_dbg("bind spider compositor version=%d id=%d\n", version, id);

	resource = wl_resource_create(client, &spider_compositor_manager_v1_interface, version, id);

	/* TODO Must check this shell is child client */
	/*
	if (client == shell->child.client)
	*/
	wl_resource_set_implementation(resource, 
			&spider_compositor_implementation, compositor, NULL);
}

static void register_spider_compositor_interface(struct spider_compositor *compositor)
{
	if (wl_global_create(compositor->wl_display,
			     &spider_compositor_manager_v1_interface, 1,
			     compositor, bind_spider_compositor) == NULL) {
		return;
	}
	spider_dbg("register spider compositor interfaces\n");
}

int init_compositor()
{
	int child_pid;

	if (g_options.verbose) {
		wlr_log_init(WLR_DEBUG, NULL);
	}else if (g_options.debug) {
		wlr_log_init(WLR_INFO, NULL);
	}else {
		wlr_log_init(WLR_ERROR, NULL);
	}

	compositor->renderer = wlr_backend_get_renderer(compositor->backend);
	wlr_renderer_init_wl_display(compositor->renderer, compositor->wl_display);

	compositor->compositor = wlr_compositor_create(compositor->wl_display, compositor->renderer);
	wlr_data_device_manager_create(compositor->wl_display);

	compositor->output_layout = wlr_output_layout_create();
	/*
	wlr_xdg_output_manager_v1_create(server->wl_display, compositor->layout);
	compositor->layout_change.notify = handle_layout_change;
	wl_signal_add(&compositor->layout->events.change, &compositor->layout_change);
	*/

	spider_list_init(&compositor->outputs);
	compositor->new_output.notify = handle_new_output;
	wl_signal_add(&compositor->backend->events.new_output, &compositor->new_output);

	spider_list_init(&compositor->views);

	/*
	compositor->xdg_shell_v6 = wlr_xdg_shell_v6_create(server->wl_display);
	compositor->xdg_shell_v6_surface.notify = handle_xdg_shell_v6_surface;
	wl_signal_add(&compositor->xdg_shell_v6->events.new_surface,
		&compositor->xdg_shell_v6_surface);
	*/

	compositor->xdg_shell = wlr_xdg_shell_create(compositor->wl_display);
	compositor->new_xdg_surface.notify = handle_new_xdg_surface;
	wl_signal_add(&compositor->xdg_shell->events.new_surface,
			&compositor->new_xdg_surface);

	compositor->layer_shell = wlr_layer_shell_v1_create(compositor->wl_display);
	compositor->layer_shell_surface.notify = handle_layer_shell_surface;
	wl_signal_add(&compositor->layer_shell->events.new_surface,
			&compositor->layer_shell_surface);

	create_cursor(compositor);


	spider_list_init(&compositor->keyboards);
	compositor->new_input.notify = handle_new_input;
	wl_signal_add(&compositor->backend->events.new_input, &compositor->new_input);

	compositor->seat = wlr_seat_create(compositor->wl_display, "seat0");
	compositor->request_cursor.notify = handle_seat_request_cursor;
	wl_signal_add(&compositor->seat->events.request_set_cursor,
			&compositor->request_cursor);

	/* custom interface */
	register_spider_compositor_interface(compositor);

	/* Add a Unix socket to the Wayland display. */
	const char *socket = wl_display_add_socket_auto(compositor->wl_display);
	if (!socket) {
		wlr_backend_destroy(compositor->backend);
		return 1;
	}

	/* Start the backend. This will enumerate outputs and inputs, become the DRM
	 * master, etc */
	if (!wlr_backend_start(compositor->backend)) {
		wlr_backend_destroy(compositor->backend);
		wl_display_destroy(compositor->wl_display);
		return 1;
	}

	/* Set the WAYLAND_DISPLAY environment variable to our socket and run the
	 * startup command if requested. */
	setenv("WAYLAND_DISPLAY", socket, true);
	if (!getenv(SPIDER_WEB_URL)) {
		setenv(SPIDER_WEB_URL, SPIDER_WEB_URL_PATH, 0);
	}

	if (g_options.server) {
		child_pid = fork();
		if (child_pid == 0) {
			spider_dbg("Launch server [%s]\n", g_options.server);
			execl("/bin/sh", "/bin/sh", "-c", g_options.server, (void *)NULL);
			exit(-1);
		}else if (child_pid < 0) {
			spider_err("Failed to fork\n");
			exit(-1);
		}

		compositor->client_server_pid = child_pid;
	}

	compositor->wl_event_loop = wl_display_get_event_loop(compositor->wl_display);
	//wl_event_loop_add_idle(compositor->wl_event_loop, launch_client, compositor);
	launch_client(compositor);

	/* Run the Wayland event loop. This does not return until you exit the
	 * compositor. Starting the backend rigged up all of the necessary event
	 * loop configuration to listen to libinput events, DRM events, generate
	 * frame events at the refresh rate, and so on. */
	spider_log("Running Wayland compositor on WAYLAND_DISPLAY=%s\n", socket);
	wl_display_run(compositor->wl_display);

	/* Once wl_display_run returns, we shut down the compositor. */
	wl_display_destroy_clients(compositor->wl_display);
	wl_display_destroy(compositor->wl_display);

	return 0;
}
