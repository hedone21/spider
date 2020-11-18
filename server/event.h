/*
 * Copyright (c) 2020, 2021 Minyoung.Go <hedone21@gmail.com>
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

#ifndef SPIDER_EVENT_H
#define SPIDER_EVENT_H

#include <stdbool.h>
#include <glib.h>
#include "iter.h"

enum spider_event_type {
    IDLE_EVENT = 0,

    NEW_CLIENT_EVENT,
    DEL_CLIENT_EVENT,

    NEW_WINDOW_EVENT,
    MAX_WINDOW_EVENT,
    MIN_WINDOW_EVENT,
    FULL_WINDOW_EVENT,
    DEL_WINDOW_EVENT,

    NUM_OF_EVENT
};

struct spider_event {
    GList *registered_events[NUM_OF_EVENT];
};

struct spider_event* spider_event_create();
bool spider_event_register(struct spider_event *event, enum spider_event_type ev_type, void *obj);
void* spider_event_get(struct spider_event *event, enum spider_event_type ev_type, unsigned int pos);
struct spider_iter* spider_event_get_iter(struct spider_event *event, enum spider_event_type ev_type);
void spider_event_free_iter(struct spider_iter **iter);
void spider_event_free(struct spider_event **event);

#endif /* SPIDER_EVENT_H */
