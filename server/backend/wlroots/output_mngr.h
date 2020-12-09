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

#ifndef SPIDER_BACKEND_WLROOTS_OUTPUT_MNGR_H
#define SPIDER_BACKEND_WLROOTS_OUTPUT_MNGR_H

#include <glib.h>
#include <wlr/backend.h>
#include "wlroots.h"
#include "common/util.h"
#include "output.h"

struct spider_wlroots_output_mngr {
    GList *outputs;

	struct wlr_output_layout *output_layout;
	struct wl_listener new_output;
};

struct spider_wlroots_output_mngr* spider_wlroots_output_mngr_new();
struct wl_listener* spider_wlroots_output_mngr_get_output_listener(struct spider_wlroots_output_mngr *mngr);
struct wlr_output_layout* spider_wlroots_output_mngr_get_output_layout(struct spider_wlroots_output_mngr *mngr);
void spider_wlroots_output_mngr_handle_new(struct wl_listener *listener, void *data);

#endif /* SPIDER_BACKEND_WLROOTS_OUTPUT_H */