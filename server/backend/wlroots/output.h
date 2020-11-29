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

#ifndef SPIDER_BACKEND_WLROOTS_OUTPUT_H
#define SPIDER_BACKEND_WLROOTS_OUTPUT_H

#include "wlroots.h"
#include "common/util.h"

struct spider_wlroots_output {
	struct spider_list link;
	struct wlr_output *wlr_output;

	struct wl_listener frame;
	struct wl_listener destroy;
	struct wl_listener enable;
	struct wl_listener mode;
	struct wl_listener transform;
	struct wl_listener present;
};

void spider_wlroots_output_handle_new(struct wl_listener *listener, void *data);

#endif /* SPIDER_BACKEND_WLROOTS_OUTPUT_H */
