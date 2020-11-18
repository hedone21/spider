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

#ifndef SPIDER_SERVER_H
#define SPIDER_SERVER_H

#include <stdbool.h>
#include "client_mngr.h"
#include "event.h"
#include "backend/backend.h"
#include "backend/server.h"

struct spider_server {
    struct spider_client_mngr *mngr; 
    struct spider_backend *backend;
    struct spider_backend_server *backend_server;
};

typedef bool (*cb_event)(struct spider_server *server, void *obj, void *data);

struct spider_server* spider_server_create();
struct spider_server* spider_server_create_with_backend(char *backend);
void spider_server_run(struct spider_server *server);
void spider_server_free(struct spider_server **server);
bool spider_server_register_event(struct spider_server *server, enum spider_event_type ev_type, cb_event cb);
void spider_server_emit_event(struct spider_server *server, enum spider_event_type ev_type, void *data);

#endif /* SPIDER_SERVER_H */