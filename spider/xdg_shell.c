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

#include "spider/desktop.h"
#include "spider/xdg_shell.h"
#include "spider/view.h"
#include "common/log.h"

static void handle_xdg_surface_map(struct wl_listener *listener, void *data)
{
	/* Called when the surface is mapped, or ready to display on-screen. */
	struct spider_view *view = wl_container_of(listener, view, map);
	spider_dbg("new %s is started\n", view->xdg_surface->toplevel->title);
	view->mapped = true;
	focus_view(view, view->xdg_surface->surface);
}

static void handle_xdg_surface_unmap(struct wl_listener *listener, void *data)
{
	/* Called when the surface is unmapped, and should no longer be shown. */
	struct spider_view *view = wl_container_of(listener, view, unmap);
	view->mapped = false;
}

static void handle_xdg_surface_destroy(struct wl_listener *listener, void *data)
{
	/* Called when the surface is destroyed and should never be shown again. */
	struct spider_view *view = wl_container_of(listener, view, destroy);
	spider_list_remove(&view->link);
	free(view);
}

static void begin_interactive(struct spider_view *view,	enum spider_cursor_mode mode, uint32_t edges)
{
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
		desktop->grab_x = desktop->cursor->x - view->box.x;
		desktop->grab_y = desktop->cursor->y - view->box.y;
	} else {
		desktop->grab_x = desktop->cursor->x + geo_box.x;
		desktop->grab_y = desktop->cursor->y + geo_box.y;
	}
	desktop->grab_width = geo_box.width;
	desktop->grab_height = geo_box.height;
	desktop->resize_edges = edges;
}

static void handle_xdg_toplevel_request_move(struct wl_listener *listener, void *data)
{
	struct spider_view *view = wl_container_of(listener, view, request_move);
	spider_dbg("Toplevel Request Move\n");
	begin_interactive(view, SPIDER_CURSOR_MOVE, 0);
	if (view->maximized)
		maximize_view(view, false);
}

static void handle_xdg_toplevel_request_resize(struct wl_listener *listener, void *data)
{
	/* This event is raised when a client would like to begin an interactive
	 * resize, typically because the user clicked on their client-side
	 * decorations. Note that a more sophisticated desktop should check the
	 * provied serial against a list of button press serials sent to this
	 * client, to prevent the client from requesting this whenever they want. */
	struct wlr_xdg_toplevel_resize_event *event = data;
	struct spider_view *view = wl_container_of(listener, view, request_resize);
	begin_interactive(view, SPIDER_CURSOR_RESIZE, event->edges);
}

static void handle_xdg_toplevel_request_maximize(struct wl_listener *listener, void *data)
{
	spider_dbg("MAXIMIZE is requested\n");
	struct spider_view *view = wl_container_of(listener, view, request_maximize);

	if (view->maximized == false) {
		wlr_xdg_toplevel_set_maximized(view->xdg_surface, true);
		maximize_view(view, true);
	}else {
		wlr_xdg_toplevel_set_maximized(view->xdg_surface, false);
		maximize_view(view, false);
	}
}

static void handle_xdg_toplevel_request_minimize(struct wl_listener *listener, void *data)
{
	spider_dbg("MINIMIZE is requested\n");
}

static void handle_xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data)
{
	spider_dbg("FULLSCREEN is requested\n");
	struct spider_view *view = wl_container_of(listener, view, request_fullscreen);
	wlr_xdg_toplevel_set_fullscreen(view->xdg_surface, true);
}

void handle_new_xdg_surface(struct wl_listener *listener, void *data)
{
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
	view->layer = LAYER_TOP;

	/* Listen to the various events it can emit */
	view->map.notify = handle_xdg_surface_map;
	wl_signal_add(&xdg_surface->events.map, &view->map);
	view->unmap.notify = handle_xdg_surface_unmap;
	wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
	view->destroy.notify = handle_xdg_surface_destroy;
	wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

	/* cotd */
	struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
	view->request_move.notify = handle_xdg_toplevel_request_move;
	wl_signal_add(&toplevel->events.request_move, &view->request_move);
	view->request_resize.notify = handle_xdg_toplevel_request_resize;
	wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
	view->request_maximize.notify = handle_xdg_toplevel_request_maximize;
	wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);
	view->request_minimize.notify = handle_xdg_toplevel_request_minimize;
	wl_signal_add(&toplevel->events.request_minimize, &view->request_minimize);
	view->request_fullscreen.notify = handle_xdg_toplevel_request_fullscreen;
	wl_signal_add(&toplevel->events.request_fullscreen, &view->request_fullscreen);

	/* Add it to the list of views. */
	spider_list_insert(&desktop->views, &view->link);
}
