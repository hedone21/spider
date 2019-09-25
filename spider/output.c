/*
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

#include <wlr/types/wlr_presentation_time.h>
#include "spider/desktop.h"
#include "spider/output.h"
#include "spider/view.h"
#include "common/log.h"

/* This function is called for every surface that needs to be rendered. */
static void render_surface(struct wlr_surface *surface,	int sx, int sy, void *data)
{
	struct render_data *rdata = data;
	struct spider_view *view = rdata->view;
	struct wlr_output *output = rdata->output;

	struct wlr_texture *texture = wlr_surface_get_texture(surface);
	if (texture == NULL) {
		return;
	}

	double ox = 0, oy = 0;
	wlr_output_layout_output_coords(
			view->desktop->output_layout, output, &ox, &oy);
	ox += view->box.x + sx, oy += view->box.y + sy;

	struct wlr_box box = {
		.x = ox * output->scale,
		.y = oy * output->scale,
		.width = surface->current.width * output->scale,
		.height = surface->current.height * output->scale,
	};

	/*
	spider_dbg("box x=%d y=%d width=%d height=%d\n", 
			box.x, box.y, box.width, box.height);
	*/

	float matrix[9];
	enum wl_output_transform transform =
		wlr_output_transform_invert(surface->current.transform);
	wlr_matrix_project_box(matrix, &box, transform, 0,
			output->transform_matrix);

	wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

	wlr_surface_send_frame_done(surface, rdata->when);
}

/* This function is called every time an output is ready to display a frame,
 * generally at the output's refresh rate (e.g. 60Hz). */
static void output_handle_frame(struct wl_listener *listener, void *data)
{
	struct spider_output *output = wl_container_of(listener, output, frame);
	struct wlr_renderer *renderer = output->desktop->renderer;

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	/* This func may print too much logs */
	// spider_dbg("Frame %s\n", output->wlr_output->name);

	if (!wlr_output_attach_render(output->wlr_output, NULL)) {
		return;
	}

	int width, height;
	wlr_output_effective_resolution(output->wlr_output, &width, &height);
	wlr_renderer_begin(renderer, width, height);

	float color[4] = {0.0, 0.0, 0.0, 1.0};
	wlr_renderer_clear(renderer, color);


	struct spider_view *view;
	spider_list_for_each_reverse(view, &output->desktop->views, link) {
		if (!view->mapped) {
			continue;
		}
		struct render_data rdata = {
			.output = output->wlr_output,
			.view = view,
			.renderer = renderer,
			.when = &now,
		};
		wlr_xdg_surface_for_each_surface(view->xdg_surface,
				render_surface, &rdata);
	}

	wlr_output_render_software_cursors(output->wlr_output, NULL);

	wlr_renderer_end(renderer);
	wlr_output_commit(output->wlr_output);
}

static void output_handle_destroy(struct wl_listener *listener, void *data)
{
	struct spider_output *output = wl_container_of(listener, output, destroy);
	spider_dbg("Terminate %s\n", output->wlr_output->name);
}

static void output_handle_enable(struct wl_listener *listener, void *data)
{
	struct spider_output *output = wl_container_of(listener, output, destroy);
	spider_dbg("Enable %s\n", output->wlr_output->name);
}

static void output_handle_mode(struct wl_listener *listener, void *data)
{
	struct spider_output *output = wl_container_of(listener, output, destroy);
	spider_dbg("Mode %s\n", output->wlr_output->name);
}

static void output_handle_transform(struct wl_listener *listener, void *data)
{
	struct spider_output *output = wl_container_of(listener, output, destroy);
	spider_dbg("Transform %s\n", output->wlr_output->name);
}

static void output_handle_present(struct wl_listener *listener, void *data) 
{
	struct spider_output *output = wl_container_of(listener, output, destroy);

	struct wlr_output_event_present *output_event = data;

	struct wlr_presentation_event event = {
		.output = output->wlr_output,
		.tv_sec = (uint64_t)output_event->when->tv_sec,
		.tv_nsec = (uint32_t)output_event->when->tv_nsec,
		.refresh = (uint32_t)output_event->refresh,
		.seq = (uint64_t)output_event->seq,
		.flags = output_event->flags,
	};

	/* This func may print too much logs */
	/*
	spider_dbg("Present %s sec=%lu nsec=%u refresh=%u seq=%lu flags=%u\n",
			output->wlr_output->name, event.tv_sec, event.tv_nsec,
			event.refresh, event.seq, event.flags);
	*/
}

/* This event is rasied by the backend when a new output (aka a display or
 * monitor) becomes available. */
void handle_new_output(struct wl_listener *listener, void *data)
{
	struct spider_desktop *desktop =
		wl_container_of(listener, desktop, new_output);
	struct wlr_output *wlr_output = data;

	spider_dbg("Add output: %s\n", wlr_output->name);
	spider_dbg("'%s %s %s' %"PRId32"mm x %"PRId32"mm\n", wlr_output->make,
			wlr_output->model, wlr_output->serial, wlr_output->phys_width,
			wlr_output->phys_height);

	if (!spider_list_empty(&wlr_output->modes)) {
		struct wlr_output_mode *mode =
			wl_container_of(wlr_output->modes.prev, mode, link);
		wlr_output_set_mode(wlr_output, mode);
	}

	struct spider_output *output = calloc(1, sizeof(struct spider_output));

	output->wlr_output = wlr_output;
	output->desktop = desktop;
	wlr_output->data = output;
	spider_list_insert(&desktop->outputs, &output->link);

	output->frame.notify = output_handle_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);
	output->destroy.notify = output_handle_destroy;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);
	output->enable.notify = output_handle_enable;
	wl_signal_add(&wlr_output->events.enable, &output->enable);
	output->mode.notify = output_handle_mode;
	wl_signal_add(&wlr_output->events.mode, &output->mode);
	output->transform.notify = output_handle_transform;
	wl_signal_add(&wlr_output->events.transform, &output->transform);
	output->present.notify = output_handle_present;
	wl_signal_add(&wlr_output->events.present, &output->present);

	/* TODO handle damage region to reduce overhead */
	/*
	output->damage_frame.notify = output_damage_handle_frame;
	wl_signal_add(&output->damage->events.frame, &output->damage_frame);
	output->damage_destroy.notify = output_damage_handle_destroy;
	wl_signal_add(&output->damage->events.destroy, &output->damage_destroy);
	*/

	wlr_output_layout_add_auto(desktop->output_layout, wlr_output);

	wlr_output_create_global(wlr_output);
}
