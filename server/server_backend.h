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

#ifndef SPIDER_SERVER_BACKEND_H
#define SPIDER_SERVER_BACKEND_H

#include <stdbool.h>
#include "server.h"

enum spider_server_backend_type {
    WLROOTS_BACKEND = 0,
    NUM_OF_BACKEND,
};

struct spider_server_backend {
    bool (*init)(struct spider_server *server);
    void (*run)(struct spider_server *server);
    void (*free)(struct spider_server *server);

    enum spider_server_backend_type backend_type;

    void *_user_data;
};

struct spider_server_backend* spider_server_backend_create(enum spider_server_backend_type type);
struct spider_server_backend* spider_server_backend_create_with_path(char *backend_path);
void spider_server_backend_free(struct spider_server_backend **backend);

#endif /* SPIDER_SERVER_BACKEND_H */
