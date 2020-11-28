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

#include <stdarg.h>
#include <stdlib.h>
#include "iter.h"
#include "server.h"
#include "backend/server.h"
#include "common/log.h"
#include "spider_assert.h"

struct spider_server* spider_server_create() {
    struct spider_server *server = NULL;
    server = calloc(1, sizeof(*server));
    spider_assert(server != NULL);

    server->mngr = spider_client_mngr_new();
    spider_assert(server->mngr != NULL);

    server->cursor = spider_cursor_new();
    spider_assert(server->cursor != NULL);

    return server;
}

void spider_server_add_backend(struct spider_server *server, struct spider_backend *backend) {
    spider_assert(server != NULL);
    spider_assert(backend != NULL);

    server->backend = backend;

    struct spider_backend_server *backend_server = backend->server;
    if (backend_server && backend_server->init) {
        backend_server->init(server, backend_server->data);
    }
}

/* This is infinite loop */
void spider_server_run(struct spider_server *server) {
    spider_assert(server != NULL);

    if (server->backend == NULL) {
        spider_err("NULL Backend\n");
        return;
    }

    struct spider_backend_server *backend_server = server->backend->server;

    if (backend_server && backend_server->run) {
        backend_server->run(server, backend_server->data);
    }
}

void spider_server_free(struct spider_server **server) {
    if (*server == NULL) {
        return;
    }
    
    if ((*server)->backend == NULL) {
        return;
    }

    struct spider_backend_server *backend_server = (*server)->backend->server;

    if (backend_server && backend_server->free) {
        backend_server->free(*server, backend_server->data);
    }
    spider_backend_free(&((*server)->backend));

    free(*server);
    *server = NULL;
}

struct spider_client_mngr* spider_server_get_client_mngr(struct spider_server *server) {
    spider_assert(server != NULL);
    spider_assert(server->mngr != NULL);

    return server->mngr;
}

struct spider_cursor* spider_server_get_cursor(struct spider_server *server) {
    spider_assert(server != NULL);
    spider_assert(server->cursor != NULL);

    return server->cursor;
}

bool spider_server_register_event(struct spider_server *server, enum spider_event_type ev_type, void *cb) {
    spider_assert(server != NULL);
    spider_assert(cb != NULL);
    spider_assert(ev_type < NUM_OF_EVENT && ev_type >= 0);

    server->events[ev_type] = g_list_append(server->events[ev_type], cb);

    return true;
}

void spider_server_emit_event(struct spider_server *server, enum spider_event_type ev_type, ...) {
    spider_assert(server != NULL);
    spider_assert(ev_type < NUM_OF_EVENT && ev_type >= 0);

    const unsigned int max_args = 3;
    unsigned int num_of_args = 0;
    void *args[max_args];

    switch(ev_type) {
        case NEW_CLIENT_EVENT:
            num_of_args = 1;
            break;
        case NEW_CLIENT_SHELL_EVENT:
            num_of_args = 1;
            break;
        case NEW_CLIENT_PANEL_EVENT:
            num_of_args = 1;
            break;
        case DEL_CLIENT_EVENT:
            num_of_args = 1;
            break;
        case NEW_WINDOW_EVENT:
            num_of_args = 1;
            break;
        case MAX_WINDOW_EVENT:
            num_of_args = 2;
            break;
        case MIN_WINDOW_EVENT:
            num_of_args = 2;
            break;
        case FULL_WINDOW_EVENT:
            num_of_args = 2;
            break;
        case MOVE_WINDOW_EVENT:
            num_of_args = 3;
            break;
        case RESIZE_WINDOW_EVENT:
            num_of_args = 3;
            break;
        case DEL_WINDOW_EVENT:
            num_of_args = 1;
            break;
        case MOVE_CURSOR_EVENT:
            num_of_args = 2;
            break;
        case ABSMOVE_CURSOR_EVENT:
            num_of_args = 2;
            break;
        case CLICK_CURSOR_EVENT:
            num_of_args = 1;
            break;
        case RENDER_EVENT:
            num_of_args = 0;
            break;
    }

    va_list ap;
    va_start(ap, ev_type);

    for (int i = 0; i < num_of_args; i++) {
        args[i] = va_arg(ap, void *);
    }
    
    va_end(ap);

    for (int i = 0;;i++) {
        void *func = g_list_nth_data(server->events[ev_type], i);
        if (func == NULL) {
            break;
        }

        switch(ev_type) {
            case NEW_CLIENT_EVENT:
                ((spider_new_client_cb)func)(server, args[0]);
                break;
            case NEW_CLIENT_SHELL_EVENT:
                ((spider_new_client_cb)func)(server, args[0]);
                break;
            case NEW_CLIENT_PANEL_EVENT:
                ((spider_new_client_cb)func)(server, args[0]);
                break;
            case DEL_CLIENT_EVENT:
                ((spider_del_client_cb)func)(server, args[0]);
                break;
            case NEW_WINDOW_EVENT:
                ((spider_new_window_cb)func)(server, args[0]);
                break;
            case MAX_WINDOW_EVENT:
                ((spider_max_window_cb)func)(server, args[0], args[1]);
                break;
            case MIN_WINDOW_EVENT:
                ((spider_min_window_cb)func)(server, args[0], args[1]);
                break;
            case FULL_WINDOW_EVENT:
                ((spider_full_window_cb)func)(server, args[0], args[1]);
                break;
            case MOVE_WINDOW_EVENT:
                ((spider_move_window_cb)func)(server, args[0], args[1], args[2]);
                break;
            case RESIZE_WINDOW_EVENT:
                ((spider_resize_window_cb)func)(server, args[0], args[1], args[2]);
                break;
            case DEL_WINDOW_EVENT:
                ((spider_del_window_cb)func)(server, args[0]);
                break;
            case MOVE_CURSOR_EVENT:
                ((spider_move_cursor_cb)func)(server, args[0], args[1]);
                break;
            case ABSMOVE_CURSOR_EVENT:
                ((spider_absmove_cursor_cb)func)(server, args[0], args[1]);
                break;
            case CLICK_CURSOR_EVENT:
                ((spider_click_cursor_cb)func)(server, args[0]);
                break;
            case RENDER_EVENT:
                ((spider_render_cb)func)(server);
                break;
        }
    }

    return;
}
