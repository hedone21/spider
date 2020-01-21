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

#include <signal.h>
#include "spider/compositor.h"
#include "spider/input.h"
#include "spider/view.h"
#include "common/log.h"

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
	wlr_seat_set_keyboard(keyboard->compositor->seat, keyboard->device);
	/* Send modifiers to the client. */
	wlr_seat_keyboard_notify_modifiers(keyboard->compositor->seat,
			&keyboard->device->keyboard->modifiers);
}

static bool handle_keybinding(struct spider_compositor *compositor, xkb_keysym_t sym) {
	/*
	 * Here we handle compositor keybindings. This is when the compositor is
	 * processing keys, rather than passing them on to the client for its own
	 * processing.
	 *
	 * This function assumes Alt is held down.
	 */
	switch (sym) {
		case XKB_KEY_Escape:
			wl_display_terminate(compositor->wl_display);
			kill(compositor->client_server_pid, SIGKILL);
			kill(compositor->client_shell_pid, SIGKILL);
			break;
		case XKB_KEY_F1:
			/* Cycle to the next view */
			if (spider_list_length(&compositor->views) < 2) {
				break;
			}
			struct spider_view *current_view = wl_container_of(
					compositor->views.next, current_view, link);
			struct spider_view *next_view = wl_container_of(
					current_view->link.next, next_view, link);
			if (!current_view || !next_view) {
				spider_err("Failed to find view\n");
				return false;
			}

			focus_view(next_view, next_view->xdg_surface->surface);

			/* Move the previous view to the end of the list */
			spider_list_remove(&current_view->link);
			spider_list_insert(compositor->views.prev, &current_view->link);
			break;
		default:
			return false;
	}
	return true;
}

static void handle_keyboard_key(struct wl_listener *listener, void *data) 
{
	/* This event is raised when a key is pressed or released. */
	struct spider_keyboard *keyboard =
		wl_container_of(listener, keyboard, key);
	struct spider_compositor *compositor = keyboard->compositor;
	struct wlr_event_keyboard_key *event = data;
	struct wlr_seat *seat = compositor->seat;

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
		 * process it as a compositor keybinding. */
		for (int i = 0; i < nsyms; i++) {
			handled = handle_keybinding(compositor, syms[i]);
		}
	}

	if (!handled) {
		/* Otherwise, we pass it along to the client. */
		wlr_seat_set_keyboard(seat, keyboard->device);
		wlr_seat_keyboard_notify_key(seat, event->time_msec,
				event->keycode, event->state);
	}
}

static void add_new_keyboard(struct spider_compositor *compositor, struct wlr_input_device *device) 
{
	struct spider_keyboard *keyboard =
		calloc(1, sizeof(struct spider_keyboard));
	keyboard->compositor = compositor;
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
	keyboard->key.notify = handle_keyboard_key;
	wl_signal_add(&device->keyboard->events.key, &keyboard->key);

	wlr_seat_set_keyboard(compositor->seat, device);

	/* And add the keyboard to our list of keyboards */
	spider_list_insert(&compositor->keyboards, &keyboard->link);
}

static void add_new_pointer(struct spider_compositor *compositor, struct wlr_input_device *device)
{
	/* We don't do anything special with pointers. All of our pointer handling
	 * is proxied through wlr_cursor. On another compositor, you might take this
	 * opportunity to do libinput configuration on the device to set
	 * acceleration, etc. */
	wlr_cursor_attach_input_device(compositor->cursor, device);
}

void handle_new_input(struct wl_listener *listener, void *data)
{
	/* This event is raised by the backend when a new input device becomes
	 * available. */
	struct spider_compositor *compositor =
		wl_container_of(listener, compositor, new_input);
	struct wlr_input_device *device = data;
	switch (device->type) {
		case WLR_INPUT_DEVICE_KEYBOARD:
			add_new_keyboard(compositor, device);
			break;
		case WLR_INPUT_DEVICE_POINTER:
			add_new_pointer(compositor, device);
			break;
		default:
			break;
	}
	/* We need to let the wlr_seat know what our capabilities are, which is
	 * communiciated to the client. In TinyWL we always have a cursor, even if
	 * there are no pointer devices, so we always include that capability. */
	uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
	if (!spider_list_empty(&compositor->keyboards)) {
		caps |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(compositor->seat, caps);
}
