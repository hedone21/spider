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

#include <stdlib.h>
#include "output_mngr.h"
#include "server/spider_assert.h"

static struct spider_wlroots_output_mngr *output_mngr = NULL;

/* This function is called every time an output is ready to display a frame,
 * generally at the output's refresh rate (e.g. 60Hz). */
static void handle_frame(struct wl_listener *listener, void *data)
{
    spider_assert(output_mngr != NULL);

	struct spider_wlroots_output *output = wl_container_of(listener, output, frame);
	struct wlr_renderer *renderer = output_mngr->renderer;
    spider_assert(renderer != NULL);

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


    /*
	struct spider_view *view;
	spider_list_for_each_reverse(view, &output->compositor->views, link) {
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
    */

	wlr_renderer_end(renderer);
	wlr_output_commit(output->wlr_output);
}

static void handle_destroy(struct wl_listener *listener, void *data)
{
	struct spider_wlroots_output *output = wl_container_of(listener, output, destroy);
	spider_dbg("Terminate %s\n", output->wlr_output->name);
}

static void handle_enable(struct wl_listener *listener, void *data)
{
	struct spider_wlroots_output *output = wl_container_of(listener, output, enable);
	spider_dbg("Enable %s\n", output->wlr_output->name);
}

static void handle_mode(struct wl_listener *listener, void *data)
{
	struct spider_wlroots_output *output = wl_container_of(listener, output, mode);
	spider_dbg("Mode %s\n", output->wlr_output->name);
}

static void handle_transform(struct wl_listener *listener, void *data)
{
	struct spider_wlroots_output *output = wl_container_of(listener, output, transform);
	spider_dbg("Transform %s\n", output->wlr_output->name);
}

static void handle_present(struct wl_listener *listener, void *data) 
{
	struct spider_wlroots_output *output = wl_container_of(listener, output, present);

	struct wlr_output_event_present *output_event = data;

    /*
	struct wlr_presentation_event event = {
		.output = output->wlr_output,
		.tv_sec = (uint64_t)output_event->when->tv_sec,
		.tv_nsec = (uint32_t)output_event->when->tv_nsec,
		.refresh = (uint32_t)output_event->refresh,
		.seq = (uint64_t)output_event->seq,
		.flags = output_event->flags,
	};
    */

	/* This func may print too much logs */
	/*
	spider_dbg("Present %s sec=%lu nsec=%u refresh=%u seq=%lu flags=%u\n",
			output->wlr_output->name, event.tv_sec, event.tv_nsec,
			event.refresh, event.seq, event.flags);
	*/
}

static struct spider_wlroots_output_mngr* spider_wlroots_output_mngr_new() {
    struct spider_wlroots_output_mngr *mngr = NULL;
    mngr = calloc(1, sizeof(*mngr));
    spider_assert(mngr != NULL);

	mngr->output_layout = wlr_output_layout_create();
	mngr->new_output.notify = spider_wlroots_output_mngr_handle_new;

    return mngr;
}

struct spider_wlroots_output_mngr* spider_wlroots_output_mngr_get_instance() {
    if (output_mngr == NULL) {
        output_mngr = spider_wlroots_output_mngr_new();
        spider_assert(output_mngr != NULL);
    }

    return output_mngr;
}

void spider_wlroots_output_set_renderer(struct spider_wlroots_output_mngr *mngr, struct wlr_renderer *renderer) {
    spider_assert(mngr != NULL);

    mngr->renderer = renderer;
}

struct wl_listener* spider_wlroots_output_mngr_get_output_listener(struct spider_wlroots_output_mngr *mngr) {
    spider_assert(mngr);
    return &mngr->new_output;
}

struct wlr_output_layout* spider_wlroots_output_mngr_get_output_layout(struct spider_wlroots_output_mngr *mngr) {
    spider_assert(mngr);
    return mngr->output_layout;
}

void spider_wlroots_output_mngr_handle_new(struct wl_listener *listener, void *data) {
    struct spider_wlroots_output_mngr *mngr = wl_container_of(listener, mngr, new_output);
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

	struct spider_wlroots_output *output = calloc(1, sizeof(struct spider_wlroots_output));

	output->wlr_output = wlr_output;
	wlr_output->data = output;
    mngr->outputs = g_list_prepend(mngr->outputs, &output->link);

	output->frame.notify = handle_frame;
	wl_signal_add(&wlr_output->events.frame, &output->frame);
	output->destroy.notify = handle_destroy;
	wl_signal_add(&wlr_output->events.destroy, &output->destroy);
	output->enable.notify = handle_enable;
	wl_signal_add(&wlr_output->events.enable, &output->enable);
	output->mode.notify = handle_mode;
	wl_signal_add(&wlr_output->events.mode, &output->mode);
	output->transform.notify = handle_transform;
	wl_signal_add(&wlr_output->events.transform, &output->transform);
	output->present.notify = handle_present;
	wl_signal_add(&wlr_output->events.present, &output->present);

	/* TODO handle damage region to reduce overhead */
	/*
	output->damage_frame.notify = output_damage_handle_frame;
	wl_signal_add(&output->damage->events.frame, &output->damage_frame);
	output->damage_destroy.notify = output_damage_handle_destroy;
	wl_signal_add(&output->damage->events.destroy, &output->damage_destroy);
	*/

	wlr_output_layout_add_auto(mngr->output_layout, wlr_output);

	wlr_output_create_global(wlr_output);
}
