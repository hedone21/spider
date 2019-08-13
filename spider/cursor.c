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

#include "spider/desktop.h"
#include "spider/cursor.h"

static void process_cursor_move(struct spider_desktop *desktop, uint32_t time) {
	/* Move the grabbed view to the new position. */
	desktop->grabbed_view->x = desktop->cursor->x - desktop->grab_x;
	desktop->grabbed_view->y = desktop->cursor->y - desktop->grab_y;
}

static void process_cursor_resize(struct spider_desktop *desktop, uint32_t time) {
	/*
	 * Resizing the grabbed view can be a little bit complicated, because we
	 * could be resizing from any corner or edge. This not only resizes the view
	 * on one or two axes, but can also move the view if you resize from the top
	 * or left edges (or top-left corner).
	 *
	 * Note that I took some shortcuts here. In a more fleshed-out desktop,
	 * you'd wait for the client to prepare a buffer at the new size, then
	 * commit any movement that was prepared.
	 */
	struct spider_view *view = desktop->grabbed_view;
	double dx = desktop->cursor->x - desktop->grab_x;
	double dy = desktop->cursor->y - desktop->grab_y;
	double x = view->x;
	double y = view->y;
	int width = desktop->grab_width;
	int height = desktop->grab_height;
	if (desktop->resize_edges & WLR_EDGE_TOP) {
		y = desktop->grab_y + dy;
		height -= dy;
		if (height < 1) {
			y += height;
		}
	} else if (desktop->resize_edges & WLR_EDGE_BOTTOM) {
		height += dy;
	}
	if (desktop->resize_edges & WLR_EDGE_LEFT) {
		x = desktop->grab_x + dx;
		width -= dx;
		if (width < 1) {
			x += width;
		}
	} else if (desktop->resize_edges & WLR_EDGE_RIGHT) {
		width += dx;
	}
	view->x = x;
	view->y = y;
	wlr_xdg_toplevel_set_size(view->xdg_surface, width, height);
}

static void process_cursor_motion(struct spider_desktop *desktop, uint32_t time) {
	/* If the mode is non-passthrough, delegate to those functions. */
	if (desktop->cursor_mode == SPIDER_CURSOR_MOVE) {
		process_cursor_move(desktop, time);
		return;
	} else if (desktop->cursor_mode == SPIDER_CURSOR_RESIZE) {
		process_cursor_resize(desktop, time);
		return;
	}

	/* Otherwise, find the view under the pointer and send the event along. */
	double sx, sy;
	struct wlr_seat *seat = desktop->seat;
	struct wlr_surface *surface = NULL;
	struct spider_view *view = desktop_view_at(desktop,
			desktop->cursor->x, desktop->cursor->y, &surface, &sx, &sy);
	if (!view) {
		/* If there's no view under the cursor, set the cursor image to a
		 * default. This is what makes the cursor image appear when you move it
		 * around the screen, not over any views. */
		wlr_xcursor_manager_set_cursor_image(
				desktop->cursor_mgr, "left_ptr", desktop->cursor);
	}
	if (surface) {
		bool focus_changed = seat->pointer_state.focused_surface != surface;
		/*
		 * "Enter" the surface if necessary. This lets the client know that the
		 * cursor has entered one of its surfaces.
		 *
		 * Note that this gives the surface "pointer focus", which is distinct
		 * from keyboard focus. You get pointer focus by moving the pointer over
		 * a window.
		 */
		wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
		if (!focus_changed) {
			/* The enter event contains coordinates, so we only need to notify
			 * on motion if the focus did not change. */
			wlr_seat_pointer_notify_motion(seat, time, sx, sy);
		}
	} else {
		/* Clear pointer focus so future button events and such are not sent to
		 * the last client to have the cursor over it. */
		wlr_seat_pointer_clear_focus(seat);
	}
}

static void desktop_cursor_motion(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a _relative_
	 * pointer motion event (i.e. a delta) */
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, cursor_motion);
	struct wlr_event_pointer_motion *event = data;
	/* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */
	wlr_cursor_move(desktop->cursor, event->device,
			event->delta_x, event->delta_y);
	process_cursor_motion(desktop, event->time_msec);
}

static void desktop_cursor_motion_absolute(
		struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an _absolute_
	 * motion event, from 0..1 on each axis. This happens, for example, when
	 * wlroots is running under a Wayland window rather than KMS+DRM, and you
	 * move the mouse over the window. You could enter the window from any edge,
	 * so we have to warp the mouse there. There is also some hardware which
	 * emits these events. */
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, cursor_motion_absolute);
	struct wlr_event_pointer_motion_absolute *event = data;
	wlr_cursor_warp_absolute(desktop->cursor, event->device, event->x, event->y);
	process_cursor_motion(desktop, event->time_msec);
}

static void desktop_cursor_button(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a button
	 * event. */
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, cursor_button);
	struct wlr_event_pointer_button *event = data;
	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(desktop->seat,
			event->time_msec, event->button, event->state);
	double sx, sy;
	struct wlr_seat *seat = desktop->seat;
	struct wlr_surface *surface;
	struct spider_view *view = desktop_view_at(desktop,
			desktop->cursor->x, desktop->cursor->y, &surface, &sx, &sy);
	if (event->state == WLR_BUTTON_RELEASED) {
		/* If you released any buttons, we exit interactive move/resize mode. */
		desktop->cursor_mode = SPIDER_CURSOR_PASSTHROUGH;
	} else {
		/* Focus that client if the button was _pressed_ */
		focus_view(view, surface);
	}
}

static void desktop_cursor_axis(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an axis event,
	 * for example when you move the scroll wheel. */
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, cursor_axis);
	struct wlr_event_pointer_axis *event = data;
	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(desktop->seat,
			event->time_msec, event->orientation, event->delta,
			event->delta_discrete, event->source);
}

static void desktop_cursor_frame(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an frame
	 * event. Frame events are sent after regular pointer events to group
	 * multiple events together. For instance, two axis events may happen at the
	 * same time, in which case a frame event won't be sent in between. */
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, cursor_frame);
	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(desktop->seat);
}

int spider_create_cursor(struct spider_desktop *desktop)
{
	/*
	 * Creates a cursor, which is a wlroots utility for tracking the cursor
	 * image shown on screen.
	 */
	desktop->cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(desktop->cursor, desktop->output_layout);

	/* Creates an xcursor manager, another wlroots utility which loads up
	 * Xcursor themes to source cursor images from and makes sure that cursor
	 * images are available at all scale factors on the screen (necessary for
	 * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
	desktop->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
	wlr_xcursor_manager_load(desktop->cursor_mgr, 1);

	/*
	 * wlr_cursor *only* displays an image on screen. It does not move around
	 * when the pointer moves. However, we can attach input devices to it, and
	 * it will generate aggregate events for all of them. In these events, we
	 * can choose how we want to process them, forwarding them to clients and
	 * moving the cursor around. More detail on this process is described in my
	 * input handling blog post:
	 *
	 * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
	 *
	 * And more comments are sprinkled throughout the notify functions above.
	 */
	desktop->cursor_motion.notify = desktop_cursor_motion;
	wl_signal_add(&desktop->cursor->events.motion, &desktop->cursor_motion);
	desktop->cursor_motion_absolute.notify = desktop_cursor_motion_absolute;
	wl_signal_add(&desktop->cursor->events.motion_absolute,
			&desktop->cursor_motion_absolute);
	desktop->cursor_button.notify = desktop_cursor_button;
	wl_signal_add(&desktop->cursor->events.button, &desktop->cursor_button);
	desktop->cursor_axis.notify = desktop_cursor_axis;
	wl_signal_add(&desktop->cursor->events.axis, &desktop->cursor_axis);
	desktop->cursor_frame.notify = desktop_cursor_frame;
	wl_signal_add(&desktop->cursor->events.frame, &desktop->cursor_frame);

	return 0;
}
