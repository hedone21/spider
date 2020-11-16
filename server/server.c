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

#include <stdlib.h>
#include "server.h"
#include "spider_assert.h"

struct spider_server* spider_server_create_with_backendpath(char *backend_path) {
    struct spider_server *server = NULL;
    server = calloc(1, sizeof(*server));
    spider_assert(server != NULL);

    server->backend = spider_backend_create_with_sopath(backend_path);
    spider_assert(server->backend);

    return server;
}

struct spider_server* spider_server_create() {
    struct spider_server *server = NULL;
    server = calloc(1, sizeof(*server));
    spider_assert(server != NULL);

    server->backend = spider_backend_create(WLROOTS_BACKEND);
    spider_assert(server->backend);

    return server;
}

/* This is infinite loop */
void spider_server_run(struct spider_server *server) {
}

void spider_server_free(struct spider_server **server) {
    spider_backend_free(&((*server)->backend));
    spider_backend_server_free(&((*server)->backend_server));
    free(*server);
    *server = NULL;
}
