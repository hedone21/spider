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

#include "output.h"
#include "server.h"

void spider_wlroots_output_handle_new(struct wl_listener *listener, void *data) {
    struct spider_wlroots_server *wlserver =
        wl_container_of(listener, struct spider_wlroots_server, new_output);
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
	spider_list_insert(&wlserver->outputs, &output->link);

    /*
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
    */

	/* TODO handle damage region to reduce overhead */
	/*
	output->damage_frame.notify = output_damage_handle_frame;
	wl_signal_add(&output->damage->events.frame, &output->damage_frame);
	output->damage_destroy.notify = output_damage_handle_destroy;
	wl_signal_add(&output->damage->events.destroy, &output->damage_destroy);
	*/

	wlr_output_layout_add_auto(wlserver->output_layout, wlr_output);

	wlr_output_create_global(wlr_output);
}
