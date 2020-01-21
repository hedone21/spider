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

#include "spider/compositor.h"
#include "spider/cursor.h"
#include "spider/view.h"
#include "common/log.h"

static void process_cursor_move(struct spider_compositor *compositor, uint32_t time) {
	/* Move the grabbed view to the new position. */
	compositor->grabbed_view->box.x = compositor->cursor->x - compositor->grab_x;
	compositor->grabbed_view->box.y = compositor->cursor->y - compositor->grab_y;
}

static void process_cursor_resize(struct spider_compositor *compositor, uint32_t time) {
	/*
	 * Resizing the grabbed view can be a little bit complicated, because we
	 * could be resizing from any corner or edge. This not only resizes the view
	 * on one or two axes, but can also move the view if you resize from the top
	 * or left edges (or top-left corner).
	 *
	 * Note that I took some shortcuts here. In a more fleshed-out compositor,
	 * you'd wait for the client to prepare a buffer at the new size, then
	 * commit any movement that was prepared.
	 */
	struct spider_view *view = compositor->grabbed_view;
	double dx = compositor->cursor->x - compositor->grab_x;
	double dy = compositor->cursor->y - compositor->grab_y;
	double x = view->box.x;
	double y = view->box.y;
	int width = compositor->grab_width;
	int height = compositor->grab_height;
	if (compositor->resize_edges & WLR_EDGE_TOP) {
		y = compositor->grab_y + dy;
		height -= dy;
		if (height < 1) {
			y += height;
		}
	} else if (compositor->resize_edges & WLR_EDGE_BOTTOM) {
		height += dy;
	}
	if (compositor->resize_edges & WLR_EDGE_LEFT) {
		x = compositor->grab_x + dx;
		width -= dx;
		if (width < 1) {
			x += width;
		}
	} else if (compositor->resize_edges & WLR_EDGE_RIGHT) {
		width += dx;
	}
	view->box.x = x;
	view->box.y = y;
	view->box.width = width;
	view->box.height = height;
	wlr_xdg_toplevel_set_size(view->xdg_surface, width, height);
}

static void process_cursor_motion(struct spider_compositor *compositor, uint32_t time) {
	/* If the mode is non-passthrough, delegate to those functions. */
	if (compositor->cursor_mode == SPIDER_CURSOR_MOVE) {
		process_cursor_move(compositor, time);
		return;
	} else if (compositor->cursor_mode == SPIDER_CURSOR_RESIZE) {
		process_cursor_resize(compositor, time);
		return;
	}

	/* Otherwise, find the view under the pointer and send the event along. */
	double sx, sy;
	struct wlr_seat *seat = compositor->seat;
	struct wlr_surface *surface = NULL;
	struct spider_view *view = compositor_view_at(compositor,
			compositor->cursor->x, compositor->cursor->y, &surface, &sx, &sy);
	if (!view) {
		/* If there's no view under the cursor, set the cursor image to a
		 * default. This is what makes the cursor image appear when you move it
		 * around the screen, not over any views. */
		wlr_xcursor_manager_set_cursor_image(
				compositor->cursor_mgr, "left_ptr", compositor->cursor);
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

static void compositor_cursor_motion(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a _relative_
	 * pointer motion event (i.e. a delta) */
	struct spider_compositor *compositor =
		wl_container_of(listener, compositor, cursor_motion);
	struct wlr_event_pointer_motion *event = data;
	/* The cursor doesn't move unless we tell it to. The cursor automatically
	 * handles constraining the motion to the output layout, as well as any
	 * special configuration applied for the specific input device which
	 * generated the event. You can pass NULL for the device if you want to move
	 * the cursor around without any input. */
	wlr_cursor_move(compositor->cursor, event->device,
			event->delta_x, event->delta_y);
	process_cursor_motion(compositor, event->time_msec);
}

static void compositor_cursor_motion_absolute(
		struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an _absolute_
	 * motion event, from 0..1 on each axis. This happens, for example, when
	 * wlroots is running under a Wayland window rather than KMS+DRM, and you
	 * move the mouse over the window. You could enter the window from any edge,
	 * so we have to warp the mouse there. There is also some hardware which
	 * emits these events. */
	struct spider_compositor *compositor =
		wl_container_of(listener, compositor, cursor_motion_absolute);
	struct wlr_event_pointer_motion_absolute *event = data;
	wlr_cursor_warp_absolute(compositor->cursor, event->device, event->x, event->y);
	process_cursor_motion(compositor, event->time_msec);
}

static void compositor_cursor_button(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits a button
	 * event. */
	struct spider_compositor *compositor =
		wl_container_of(listener, compositor, cursor_button);
	struct wlr_event_pointer_button *event = data;
	/* Notify the client with pointer focus that a button press has occurred */
	wlr_seat_pointer_notify_button(compositor->seat,
			event->time_msec, event->button, event->state);
	double sx, sy;
	struct wlr_seat *seat = compositor->seat;
	struct wlr_surface *surface;
	struct spider_view *view = compositor_view_at(compositor,
			compositor->cursor->x, compositor->cursor->y, &surface, &sx, &sy);
	if (!view) {
		spider_err("Failed to find cursor view\n");
		return;
	}

	if (event->state == WLR_BUTTON_RELEASED) {
		/* If you released any buttons, we exit interactive move/resize mode. */
		compositor->cursor_mode = SPIDER_CURSOR_PASSTHROUGH;
	} else {
		/* Focus that client if the button was _pressed_ */
		focus_view(view, surface);
	}
}

static void compositor_cursor_axis(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an axis event,
	 * for example when you move the scroll wheel. */
	struct spider_compositor *compositor =
		wl_container_of(listener, compositor, cursor_axis);
	struct wlr_event_pointer_axis *event = data;
	/* Notify the client with pointer focus of the axis event. */
	wlr_seat_pointer_notify_axis(compositor->seat,
			event->time_msec, event->orientation, event->delta,
			event->delta_discrete, event->source);
}

static void compositor_cursor_frame(struct wl_listener *listener, void *data) {
	/* This event is forwarded by the cursor when a pointer emits an frame
	 * event. Frame events are sent after regular pointer events to group
	 * multiple events together. For instance, two axis events may happen at the
	 * same time, in which case a frame event won't be sent in between. */
	struct spider_compositor *compositor =
		wl_container_of(listener, compositor, cursor_frame);
	/* Notify the client with pointer focus of the frame event. */
	wlr_seat_pointer_notify_frame(compositor->seat);
}

int create_cursor(struct spider_compositor *compositor)
{
	/*
	 * Creates a cursor, which is a wlroots utility for tracking the cursor
	 * image shown on screen.
	 */
	compositor->cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(compositor->cursor, compositor->output_layout);

	/* Creates an xcursor manager, another wlroots utility which loads up
	 * Xcursor themes to source cursor images from and makes sure that cursor
	 * images are available at all scale factors on the screen (necessary for
	 * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
	compositor->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
	wlr_xcursor_manager_load(compositor->cursor_mgr, 1);

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
	compositor->cursor_motion.notify = compositor_cursor_motion;
	wl_signal_add(&compositor->cursor->events.motion, &compositor->cursor_motion);
	compositor->cursor_motion_absolute.notify = compositor_cursor_motion_absolute;
	wl_signal_add(&compositor->cursor->events.motion_absolute,
			&compositor->cursor_motion_absolute);
	compositor->cursor_button.notify = compositor_cursor_button;
	wl_signal_add(&compositor->cursor->events.button, &compositor->cursor_button);
	compositor->cursor_axis.notify = compositor_cursor_axis;
	wl_signal_add(&compositor->cursor->events.axis, &compositor->cursor_axis);
	compositor->cursor_frame.notify = compositor_cursor_frame;
	wl_signal_add(&compositor->cursor->events.frame, &compositor->cursor_frame);

	return 0;
}
