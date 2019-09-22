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

#include "spider/layer.h"
#include "spider/view.h"

static struct wlr_output *get_output_from_view(struct spider_view *view)
{
	double output_x, output_y;

	wlr_output_layout_closest_point(view->desktop->output_layout, NULL,
			view->box.x + (double)view->box.width/2,
			view->box.y + (double)view->box.height/2,
			&output_x, &output_y);

	return wlr_output_layout_output_at(view->desktop->output_layout, output_x, output_y);
}

void maximize_view(struct spider_view *view, bool maximized)
{
	if (view->maximized == maximized)
		return;

	wlr_xdg_toplevel_set_maximized(view->xdg_surface, maximized); 

	if (!view->maximized && maximized) {
		view->maximized = true;
		view->saved.x = view->box.x;
		view->saved.y = view->box.y;
		view->saved.width = view->box.width;
		view->saved.height = view->box.height;

		struct wlr_output *output = get_output_from_view(view);
		struct wlr_box *output_box = wlr_output_layout_get_box(view->desktop->output_layout, output);

		view->box.x = output_box->x;
		view->box.y = output_box->y;
		view->box.width = output_box->width;
		view->box.height = output_box->height;

		wlr_xdg_toplevel_set_size(view->xdg_surface, output_box->width, output_box->height);
	}else if (view->maximized && !maximized) {
		view->maximized = false;
		view->box.x = view->saved.x;
		view->box.y = view->saved.y;
		view->box.width = view->saved.width;
		view->box.height = view->saved.height;

		wlr_xdg_toplevel_set_size(view->xdg_surface, view->box.width, view->box.height);
	}
}

void focus_view(struct spider_view *view, struct wlr_surface *surface)
{
	/* Note: this function only deals with keyboard focus. */
	struct spider_desktop *desktop = view->desktop;
	struct wlr_seat *seat = desktop->seat;
	struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;

	if (view == NULL) {
		return;
	}
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
	if (view->layer != LAYER_BACKGROUND ||
			view->xdg_surface->toplevel->server_pending.activated) {
		/* Move the view to the front */
		wl_list_remove(&view->link);
		wl_list_insert(&desktop->views, &view->link);
	}
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

static bool view_at(struct spider_view *view,
		double lx, double ly, struct wlr_surface **surface,
		double *sx, double *sy)
{
	/*
	 * XDG toplevels may have nested surfaces, such as popup windows for context
	 * menus or tooltips. This function tests if any of those are underneath the
	 * coordinates lx and ly (in output Layout Coordinates). If so, it sets the
	 * surface pointer to that wlr_surface and the sx and sy coordinates to the
	 * coordinates relative to that surface's top-left corner.
	 */
	double view_sx = lx - view->box.x;
	double view_sy = ly - view->box.y;

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
		struct wlr_surface **surface, double *sx, double *sy)
{
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
