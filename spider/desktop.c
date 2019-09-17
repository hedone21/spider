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
#include "spider/desktop.h"
#include "spider/cursor.h"
#include "spider/input.h"
#include "spider/launcher.h"
#include "spider/layer.h"
#include "spider/seat.h"
#include "spider/view.h"
#include "spider/xdg_shell.h"
#include "common/log.h"
#include "common/global_vars.h"
#include "protocol/spider-desktop-manager-v1-protocol.h"

static struct spider_desktop *desktop;

int preinit_desktop()
{
	desktop = calloc(1, sizeof(*desktop));
	if (desktop == NULL) {
		spider_err("Allocation Failed\n");
		return -1;
	}
	/* The Wayland display is managed by libwayland. It handles accepting
	 * clients from the Unix socket, manging Wayland globals, and so on. */
	desktop->wl_display = wl_display_create();
	/* The backend is a wlroots feature which abstracts the underlying input and
	 * output hardware. The autocreate option will choose the most suitable
	 * backend based on the current environment, such as opening an X11 window
	 * if an X11 desktop is running. The NULL argument here optionally allows you
	 * to pass in a custom renderer if wlr_renderer doesn't meet your needs. The
	 * backend uses the renderer, for example, to fall back to software cursors
	 * if the backend does not support hardware cursors (some older GPUs
	 * don't). */
	desktop->backend = wlr_backend_autocreate(desktop->wl_display, NULL);
	desktop->noop_backend = wlr_noop_backend_create(desktop->wl_display);

	if (!desktop->backend) {
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
	static bool is_background_set = false;

	if (is_background_set)
		return;

	struct spider_view *view;
	wl_list_for_each(view, &desktop->views, link) {
		if (view->xdg_surface->surface == wlr_surface) {
			spider_dbg("set background %p / %p\n", view, wlr_surface);
			view->layer = LAYER_BACKGROUND;
			is_background_set = true;
		}
	}
}

static const struct spider_desktop_manager_v1_interface spider_desktop_implementation = {
	.set_background = set_background,
};

static void bind_spider_desktop(struct wl_client *client,
		   void *data, uint32_t version, uint32_t id)
{
	struct spider_desktop *desktop = data;
	struct wl_resource *resource;
	spider_dbg("bind spider desktop version=%d id=%d\n", version, id);

	resource = wl_resource_create(client, &spider_desktop_manager_v1_interface, version, id);

	/* TODO Must check this shell is child client */
	/*
	if (client == shell->child.client)
	*/
	wl_resource_set_implementation(resource, 
			&spider_desktop_implementation, desktop, NULL);
}

static void register_spider_desktop_interface(struct spider_desktop *desktop)
{
	if (wl_global_create(desktop->wl_display,
			     &spider_desktop_manager_v1_interface, 1,
			     desktop, bind_spider_desktop) == NULL) {
		return;
	}
	spider_dbg("register spider desktop interfaces\n");
}

int init_desktop()
{
	int child_pid;

	if (g_options.verbose) {
		wlr_log_init(WLR_DEBUG, NULL);
	}else if (g_options.debug) {
		wlr_log_init(WLR_INFO, NULL);
	}else {
		wlr_log_init(WLR_ERROR, NULL);
	}

	desktop->renderer = wlr_backend_get_renderer(desktop->backend);
	wlr_renderer_init_wl_display(desktop->renderer, desktop->wl_display);

	desktop->compositor = wlr_compositor_create(desktop->wl_display, desktop->renderer);
	wlr_data_device_manager_create(desktop->wl_display);

	desktop->output_layout = wlr_output_layout_create();
	/*
	wlr_xdg_output_manager_v1_create(server->wl_display, desktop->layout);
	desktop->layout_change.notify = handle_layout_change;
	wl_signal_add(&desktop->layout->events.change, &desktop->layout_change);
	*/

	wl_list_init(&desktop->outputs);
	desktop->new_output.notify = handle_new_output;
	wl_signal_add(&desktop->backend->events.new_output, &desktop->new_output);

	wl_list_init(&desktop->views);

	/*
	desktop->xdg_shell_v6 = wlr_xdg_shell_v6_create(server->wl_display);
	desktop->xdg_shell_v6_surface.notify = handle_xdg_shell_v6_surface;
	wl_signal_add(&desktop->xdg_shell_v6->events.new_surface,
		&desktop->xdg_shell_v6_surface);
	*/

	desktop->xdg_shell = wlr_xdg_shell_create(desktop->wl_display);
	desktop->new_xdg_surface.notify = handle_new_xdg_surface;
	wl_signal_add(&desktop->xdg_shell->events.new_surface,
			&desktop->new_xdg_surface);

	desktop->layer_shell = wlr_layer_shell_v1_create(desktop->wl_display);
	desktop->layer_shell_surface.notify = handle_layer_shell_surface;
	wl_signal_add(&desktop->layer_shell->events.new_surface,
			&desktop->layer_shell_surface);

	create_cursor(desktop);


	wl_list_init(&desktop->keyboards);
	desktop->new_input.notify = handle_new_input;
	wl_signal_add(&desktop->backend->events.new_input, &desktop->new_input);

	desktop->seat = wlr_seat_create(desktop->wl_display, "seat0");
	desktop->request_cursor.notify = handle_seat_request_cursor;
	wl_signal_add(&desktop->seat->events.request_set_cursor,
			&desktop->request_cursor);

	/* custom interface */
	register_spider_desktop_interface(desktop);

	/* Add a Unix socket to the Wayland display. */
	const char *socket = wl_display_add_socket_auto(desktop->wl_display);
	if (!socket) {
		wlr_backend_destroy(desktop->backend);
		return 1;
	}

	/* Start the backend. This will enumerate outputs and inputs, become the DRM
	 * master, etc */
	if (!wlr_backend_start(desktop->backend)) {
		wlr_backend_destroy(desktop->backend);
		wl_display_destroy(desktop->wl_display);
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

		desktop->client_server_pid = child_pid;
	}

	desktop->wl_event_loop = wl_display_get_event_loop(desktop->wl_display);
	if (g_options.shell) {
		wl_event_loop_add_idle(desktop->wl_event_loop, launch_client, desktop);
	}

	if (g_options.panel) {
		child_pid = fork();
		if (child_pid == 0) {
			spider_dbg("Launch panel [%s]\n", g_options.panel);
			execl("/bin/sh", "/bin/sh", "-c", g_options.panel, (void *)NULL);
			exit(-1);
		}else if (child_pid < 0) {
			spider_err("Failed to fork\n");
			exit(-1);
		}

		desktop->client_server_pid = child_pid;
	}

	/* Run the Wayland event loop. This does not return until you exit the
	 * desktop. Starting the backend rigged up all of the necessary event
	 * loop configuration to listen to libinput events, DRM events, generate
	 * frame events at the refresh rate, and so on. */
	spider_log("Running Wayland desktop on WAYLAND_DISPLAY=%s\n", socket);
	wl_display_run(desktop->wl_display);

	/* Once wl_display_run returns, we shut down the desktop. */
	wl_display_destroy_clients(desktop->wl_display);
	wl_display_destroy(desktop->wl_display);

	return 0;
}
