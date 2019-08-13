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

#define _POSIX_C_SOURCE 200112L
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "spider/desktop.h"
#include "spider/cursor.h"
#include "common/log.h"
#include "common/global_vars.h"

void focus_view(struct spider_view *view, struct wlr_surface *surface) {
	/* Note: this function only deals with keyboard focus. */
	if (view == NULL) {
		return;
	}
	struct spider_desktop *desktop = view->desktop;
	struct wlr_seat *seat = desktop->seat;
	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
	if (prev_surface == surface) {
		/* Don't re-focus an already focused surface. */
		return;
	}
	if (prev_surface) {
		/*
		 * Deactivate the previously focused surface. This lets the client know
		 * it no longer has focus and the client will repaint accordingly, e.g.
		 * stop displaying a caret.
		 */
		struct wlr_xdg_surface *previous = wlr_xdg_surface_from_wlr_surface(
				seat->keyboard_state.focused_surface);
		wlr_xdg_toplevel_set_activated(previous, false);
	}
	struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
	/* Move the view to the front */
	wl_list_remove(&view->link);
	wl_list_insert(&desktop->views, &view->link);
	/* Activate the new surface */
	wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
	/*
	 * Tell the seat to have the keyboard enter this surface. wlroots will keep
	 * track of this and automatically send key events to the appropriate
	 * clients without additional work on your part.
	 */
	wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface,
			keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

static void keyboard_handle_modifiers(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a modifier key, such as shift or alt, is
	 * pressed. We simply communicate this to the client. */
	struct spider_keyboard *keyboard =
		wl_container_of(listener, keyboard, modifiers);
	/*
	 * A seat can only have one keyboard, but this is a limitation of the
	 * Wayland protocol - not wlroots. We assign all connected keyboards to the
	 * same seat. You can swap out the underlying wlr_keyboard like this and
	 * wlr_seat handles this transparently.
	 */
	wlr_seat_set_keyboard(keyboard->desktop->seat, keyboard->device);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(keyboard->desktop->seat,
			&keyboard->device->keyboard->modifiers);
}

static bool handle_keybinding(struct spider_desktop *desktop, xkb_keysym_t sym) {
	/*
	 * Here we handle desktop keybindings. This is when the desktop is
	 * processing keys, rather than passing them on to the client for its own
	 * processing.
	 *
	 * This function assumes Alt is held down.
	 */
	switch (sym) {
		case XKB_KEY_Escape:
			wl_display_terminate(desktop->wl_display);
			kill(desktop->client_server_pid, SIGKILL);
			kill(desktop->client_shell_pid, SIGKILL);
			break;
		case XKB_KEY_F1:
			/* Cycle to the next view */
			if (wl_list_length(&desktop->views) < 2) {
				break;
			}
			struct spider_view *current_view = wl_container_of(
					desktop->views.next, current_view, link);
			struct spider_view *next_view = wl_container_of(
					current_view->link.next, next_view, link);
			focus_view(next_view, next_view->xdg_surface->surface);
			/* Move the previous view to the end of the list */
			wl_list_remove(&current_view->link);
			wl_list_insert(desktop->views.prev, &current_view->link);
			break;
		default:
			return false;
	}
	return true;
}

static void keyboard_handle_key(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a key is pressed or released. */
	struct spider_keyboard *keyboard =
		wl_container_of(listener, keyboard, key);
	struct spider_desktop *desktop = keyboard->desktop;
	struct wlr_event_keyboard_key *event = data;
	struct wlr_seat *seat = desktop->seat;

	/* Translate libinput keycode -> xkbcommon */
	uint32_t keycode = event->keycode + 8;
	/* Get a list of keysyms based on the keymap for this keyboard */
	const xkb_keysym_t *syms;
	int nsyms = xkb_state_key_get_syms(
			keyboard->device->keyboard->xkb_state, keycode, &syms);

	bool handled = false;
	uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);
	if ((modifiers & WLR_MODIFIER_ALT) && event->state == WLR_KEY_PRESSED) {
		/* If alt is held down and this button was _pressed_, we attempt to
		 * process it as a desktop keybinding. */
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(desktop, syms[i]);
		}
	}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(seat, keyboard->device);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
				event->keycode, event->state);
	}
}

static void desktop_new_keyboard(struct spider_desktop *desktop,
		struct wlr_input_device *device) {
	struct spider_keyboard *keyboard =
		calloc(1, sizeof(struct spider_keyboard));
	keyboard->desktop = desktop;
	keyboard->device = device;

	/* We need to prepare an XKB keymap and assign it to the keyboard. This
	 * assumes the defaults (e.g. layout = "us"). */
	struct xkb_rule_names rules = { 0 };
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules,
			XKB_KEYMAP_COMPILE_NO_FLAGS);

	wlr_keyboard_set_keymap(device->keyboard, keymap);
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

	/* Here we set up listeners for keyboard events. */
	keyboard->modifiers.notify = keyboard_handle_modifiers;
	wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);
	keyboard->key.notify = keyboard_handle_key;
	wl_signal_add(&device->keyboard->events.key, &keyboard->key);

	wlr_seat_set_keyboard(desktop->seat, device);

	/* And add the keyboard to our list of keyboards */
	wl_list_insert(&desktop->keyboards, &keyboard->link);
}

static void desktop_new_pointer(struct spider_desktop *desktop,
		struct wlr_input_device *device) {
	/* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another desktop, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */
	wlr_cursor_attach_input_device(desktop->cursor, device);
}

static void desktop_new_input(struct wl_listener *listener, void *data) {
	/* This event is raised by the backend when a new input device becomes
	 * available. */
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, new_input);
	struct wlr_input_device *device = data;
	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			desktop_new_keyboard(desktop, device);
			break;
		case WLR_INPUT_DEVICE_POINTER:
			desktop_new_pointer(desktop, device);
			break;
		default:
			break;
	}
	/* We need to let the wlr_seat know what our capabilities are, which is
	 * communiciated to the client. In TinyWL we always have a cursor, even if
	 * there are no pointer devices, so we always include that capability. */
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!wl_list_empty(&desktop->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(desktop->seat, caps);
}

static void seat_request_cursor(struct wl_listener *listener, void *data) {
	struct spider_desktop *desktop = wl_container_of(
			listener, desktop, request_cursor);
	/* This event is rasied by the seat when a client provides a cursor image */
	struct wlr_seat_pointer_request_set_cursor_event *event = data;
	struct wlr_seat_client *focused_client =
		desktop->seat->pointer_state.focused_client;
	/* This can be sent by any client, so we check to make sure this one is
	 * actually has pointer focus first. */
	if (focused_client == event->seat_client) {
		/* Once we've vetted the client, we can tell the cursor to use the
		 * provided surface as the cursor image. It will set the hardware cursor
		 * on the output that it's currently on and continue to do so as the
		 * cursor moves between outputs. */
		wlr_cursor_set_surface(desktop->cursor, event->surface,
				event->hotspot_x, event->hotspot_y);
	}
}

static bool view_at(struct spider_view *view,
		double lx, double ly, struct wlr_surface **surface,
		double *sx, double *sy) {
	/*
	 * XDG toplevels may have nested surfaces, such as popup windows for context
	 * menus or tooltips. This function tests if any of those are underneath the
	 * coordinates lx and ly (in output Layout Coordinates). If so, it sets the
	 * surface pointer to that wlr_surface and the sx and sy coordinates to the
	 * coordinates relative to that surface's top-left corner.
	 */
	double view_sx = lx - view->x;
	double view_sy = ly - view->y;

	struct wlr_surface_state *state = &view->xdg_surface->surface->current;

	double _sx, _sy;
	struct wlr_surface *_surface = NULL;
	_surface = wlr_xdg_surface_surface_at(
			view->xdg_surface, view_sx, view_sy, &_sx, &_sy);

	if (_surface != NULL) {
		*sx = _sx;
		*sy = _sy;
		*surface = _surface;
		return true;
	}

	return false;
}

struct spider_view *desktop_view_at(
		struct spider_desktop *desktop, double lx, double ly,
		struct wlr_surface **surface, double *sx, double *sy) {
	/* This iterates over all of our surfaces and attempts to find one under the
	 * cursor. This relies on desktop->views being ordered from top-to-bottom. */
	struct spider_view *view;
	wl_list_for_each(view, &desktop->views, link) {
		if (view_at(view, lx, ly, surface, sx, sy)) {
			return view;
		}
	}
	return NULL;
}

static void xdg_surface_map(struct wl_listener *listener, void *data) {
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct spider_view *view = wl_container_of(listener, view, map);
	view->mapped = true;
	focus_view(view, view->xdg_surface->surface);
}

static void xdg_surface_unmap(struct wl_listener *listener, void *data) {
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct spider_view *view = wl_container_of(listener, view, unmap);
	view->mapped = false;
}

static void xdg_surface_destroy(struct wl_listener *listener, void *data) {
	/* Called when the surface is destroyed and should never be shown again. */
	struct spider_view *view = wl_container_of(listener, view, destroy);
	wl_list_remove(&view->link);
	free(view);
}

static void begin_interactive(struct spider_view *view,
		enum spider_cursor_mode mode, uint32_t edges) {
	/* This function sets up an interactive move or resize operation, where the
	 * desktop stops propegating pointer events to clients and instead
	 * consumes them itself, to move or resize windows. */
	struct spider_desktop *desktop = view->desktop;
	struct wlr_surface *focused_surface =
		desktop->seat->pointer_state.focused_surface;
	if (view->xdg_surface->surface != focused_surface) {
		/* Deny move/resize requests from unfocused clients. */
		return;
	}
	desktop->grabbed_view = view;
	desktop->cursor_mode = mode;
	struct wlr_box geo_box;
	wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);
	if (mode == SPIDER_CURSOR_MOVE) {
		desktop->grab_x = desktop->cursor->x - view->x;
		desktop->grab_y = desktop->cursor->y - view->y;
	} else {
		desktop->grab_x = desktop->cursor->x + geo_box.x;
		desktop->grab_y = desktop->cursor->y + geo_box.y;
	}
	desktop->grab_width = geo_box.width;
	desktop->grab_height = geo_box.height;
	desktop->resize_edges = edges;
}

static void xdg_toplevel_request_move(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a client would like to begin an interactive
	 * move, typically because the user clicked on their client-side
	 * decorations. Note that a more sophisticated desktop should check the
	 * provied serial against a list of button press serials sent to this
	 * client, to prevent the client from requesting this whenever they want. */
	struct spider_view *view = wl_container_of(listener, view, request_move);
	begin_interactive(view, SPIDER_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(
		struct wl_listener *listener, void *data) {
	/* This event is raised when a client would like to begin an interactive
	 * resize, typically because the user clicked on their client-side
	 * decorations. Note that a more sophisticated desktop should check the
	 * provied serial against a list of button press serials sent to this
	 * client, to prevent the client from requesting this whenever they want. */
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct spider_view *view = wl_container_of(listener, view, request_resize);
	begin_interactive(view, SPIDER_CURSOR_RESIZE, event->edges);
}

static void xdg_toplevel_request_maximize(
		struct wl_listener *listener, void *data) {
	spider_dbg("MAXIMIZE is requested\n");
	struct spider_view *view = wl_container_of(listener, view, request_maximize);
	wlr_xdg_toplevel_set_maximized(view->xdg_surface, true);
}

static void xdg_toplevel_request_minimize(
		struct wl_listener *listener, void *data) {
	spider_dbg("MINIMIZE is requested\n");
}

static void xdg_toplevel_request_fullscreen(
		struct wl_listener *listener, void *data) {
	spider_dbg("FULLSCREEN is requested\n");
	struct spider_view *view = wl_container_of(listener, view, request_fullscreen);
	wlr_xdg_toplevel_set_fullscreen(view->xdg_surface, true);
}

static void new_xdg_surface(struct wl_listener *listener, void *data) {
	/* This event is raised when wlr_xdg_shell receives a new xdg surface from a
	 * client, either a toplevel (application window) or popup. */
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, new_xdg_surface);
	struct wlr_xdg_surface *xdg_surface = data;
	if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
		return;
	}

	/* Allocate a spider_view for this surface */
	struct spider_view *view =
		calloc(1, sizeof(struct spider_view));
	view->desktop = desktop;
	view->xdg_surface = xdg_surface;

	/* Listen to the various events it can emit */
	view->map.notify = xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	/* cotd */
	struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
	view->request_move.notify = xdg_toplevel_request_move;
	wl_signal_add(&toplevel->events.request_move, &view->request_move);
	view->request_resize.notify = xdg_toplevel_request_resize;
	wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
	view->request_maximize.notify = xdg_toplevel_request_maximize;
	wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);
	view->request_minimize.notify = xdg_toplevel_request_minimize;
	wl_signal_add(&toplevel->events.request_minimize, &view->request_minimize);
	view->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
	wl_signal_add(&toplevel->events.request_fullscreen, &view->request_fullscreen);

	/* Add it to the list of views. */
	wl_list_insert(&desktop->views, &view->link);
}

static void handle_layer_shell_surface(struct wl_listener *listener, void *data)
{
	struct wlr_layer_surface_v1 *layer_surface = data;
	spider_dbg("new layer surface: namespace %s layer %d anchor %d "
			"size %dx%d margin %d,%d,%d,%d",
			layer_surface->namespace, layer_surface->layer, layer_surface->layer,
			layer_surface->client_pending.desired_width,
			layer_surface->client_pending.desired_height,
			layer_surface->client_pending.margin.top,
			layer_surface->client_pending.margin.right,
			layer_surface->client_pending.margin.bottom,
			layer_surface->client_pending.margin.left);
}

int spider_preinit_desktop(struct spider_desktop *desktop)
{
	/* The Wayland display is managed by libwayland. It handles accepting
	 * clients from the Unix socket, manging Wayland globals, and so on. */
	desktop->wl_display = wl_display_create();
	desktop->wl_event_loop = wl_display_get_event_loop(desktop->wl_display);
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

int spider_init_desktop(struct spider_desktop *desktop)
{
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
	desktop->xdg_shell_v6_surface.notify = handle_xdg_shell_v6_surface;
	desktop->xdg_shell_v6 = wlr_xdg_shell_v6_create(server->wl_display);
	wl_signal_add(&desktop->xdg_shell_v6->events.new_surface,
		&desktop->xdg_shell_v6_surface);
	*/

	desktop->xdg_shell = wlr_xdg_shell_create(desktop->wl_display);
	desktop->new_xdg_surface.notify = new_xdg_surface;
	wl_signal_add(&desktop->xdg_shell->events.new_surface,
			&desktop->new_xdg_surface);

	desktop->layer_shell = wlr_layer_shell_v1_create(desktop->wl_display);
	desktop->layer_shell_surface.notify = handle_layer_shell_surface;
	wl_signal_add(&desktop->layer_shell->events.new_surface,
			&desktop->layer_shell_surface);

	spider_create_cursor(desktop);


	wl_list_init(&desktop->keyboards);
	desktop->new_input.notify = desktop_new_input;
	wl_signal_add(&desktop->backend->events.new_input, &desktop->new_input);
	desktop->seat = wlr_seat_create(desktop->wl_display, "seat0");
	desktop->request_cursor.notify = seat_request_cursor;
	wl_signal_add(&desktop->seat->events.request_set_cursor,
			&desktop->request_cursor);


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
		if (fork() == 0) {
			execl("/bin/sh", "/bin/sh", "-c", g_options.server, (void *)NULL);
		}
	}

	if (g_options.shell) {
		if (fork() == 0) {
			execl("/bin/sh", "/bin/sh", "-c", g_options.shell, (void *)NULL);
		}
	}
	/* Run the Wayland event loop. This does not return until you exit the
	 * desktop. Starting the backend rigged up all of the necessary event
	 * loop configuration to listen to libinput events, DRM events, generate
	 * frame events at the refresh rate, and so on. */
	wlr_log(WLR_INFO, "Running Wayland desktop on WAYLAND_DISPLAY=%s",
			socket);
	wl_display_run(desktop->wl_display);

	/* Once wl_display_run returns, we shut down the desktop. */
	wl_display_destroy_clients(desktop->wl_display);
	wl_display_destroy(desktop->wl_display);

	return 0;
}
