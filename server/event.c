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

#include "event.h"
#include "spider_assert.h"
#include "common/log.h"

static bool check_event_type(enum spider_event_type ev_type);
static unsigned int get_events_cnt(struct spider_event *event, enum spider_event_type ev_type);
static struct spider_iter* create_new_iter(struct spider_event *event, enum spider_event_type ev_type);
static struct spider_iter* get_iter_next(struct spider_iter *iter);

static bool check_event_type(enum spider_event_type ev_type) {
    if (ev_type >= NUM_OF_EVENT) {
        spider_err("Unsupported event type (%d)\n", ev_type);
        return false;
    }

    return true;
}

static unsigned int get_events_cnt(struct spider_event *event, enum spider_event_type ev_type) {
    spider_assert(event);
    if (check_event_type(ev_type) == false) {
        return 0;
    }

    unsigned int cnt = 0;
    cnt = g_list_length(event->registered_events[ev_type]);

    return cnt;
}

struct iter_pack {
    struct spider_event *event;
    enum spider_event_type ev_type;
};

static struct spider_iter* create_new_iter(struct spider_event *event, enum spider_event_type ev_type) {
    spider_assert(event);
    if (check_event_type(ev_type) == false) {
        return NULL;
    }

    struct spider_iter *iter = NULL;
    iter = calloc(1, sizeof(*iter));
    spider_assert(iter);

    struct iter_pack *pack = calloc(1, sizeof(*pack));
    spider_assert(pack);

    pack->event = event;
    pack->ev_type = ev_type;

    iter->_user_data = pack;
    iter->data = spider_event_get(event, ev_type, 0);
    iter->next = get_iter_next;

    return iter;
}

static struct spider_iter* get_iter_next(struct spider_iter *iter) {
    spider_assert(iter);

    struct iter_pack *pack = iter->_user_data;
    struct spider_event *event = pack->event;
    enum spider_event_type ev_type = pack->ev_type;

    struct spider_iter *ret_iter = create_new_iter(event, ev_type);
    spider_assert(ret_iter);

    ret_iter->pos = iter->pos + 1;
    if (ret_iter->pos >= get_events_cnt(event, ev_type)) {
        spider_event_free_iter(&ret_iter);
        ret_iter = NULL;
        goto out;
    }

    ret_iter->data = spider_event_get(event, ev_type, ret_iter->pos);
    spider_assert(ret_iter->data);

out:
    spider_event_free_iter(&iter);

    return ret_iter;
}

struct spider_event* spider_event_create() {
    struct spider_event *event = calloc(1, sizeof(*event));
    spider_assert(event != NULL);

    return event;
}

bool spider_event_register(struct spider_event *event, enum spider_event_type ev_type, void *obj) {
    spider_assert(event != NULL);
    if (check_event_type(ev_type) == false) {
        return false;
    }

    event->registered_events[ev_type] = g_list_append(event->registered_events[ev_type], obj);
    
    return true;
}

void* spider_event_get(struct spider_event *event, enum spider_event_type ev_type, unsigned int pos) {
    spider_assert(event != NULL);
    if (check_event_type(ev_type) == false) {
        return NULL;
    }

    return g_list_nth_data(event->registered_events[ev_type], pos);
}

struct spider_iter* spider_event_get_iter(struct spider_event *event, enum spider_event_type ev_type) {
    spider_assert(event != NULL);
    if (check_event_type(ev_type) == false) {
        return NULL;
    }

    struct spider_iter *iter = create_new_iter(event, ev_type);

    return iter;
}

void spider_event_free_iter(struct spider_iter **iter) {
    if (*iter == NULL) {
        return;
    }

    free((*iter)->_user_data);
    free(*iter);
}

void spider_event_free(struct spider_event **event) {
    if (*event == NULL) {
        return;
    } 

    for (int i = 0; i < NUM_OF_EVENT; i++) {
        g_list_free((*event)->registered_events[i]);
    }

    free(*event);
    *event = NULL;
}
