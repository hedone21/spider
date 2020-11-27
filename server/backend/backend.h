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

#ifndef SPIDER_BACKEND_H
#define SPIDER_BACKEND_H

enum spider_backend_type {
    WLROOTS_BACKEND = 0,
    NUM_OF_BACKEND,
};

struct spider_backend {
    void *handle;
    enum spider_backend_type type;

    struct spider_backend_server *server;
};

struct spider_backend* spider_backend_create(enum spider_backend_type type);
struct spider_backend* spider_backend_create_with_sopath(const char *path);
void* spider_backend_get_sym(struct spider_backend *backend, const char *sym);
char* spider_backend_get_path(enum spider_backend_type type);
void spider_backend_free(struct spider_backend **backend);

#endif /* SPIDER_BACKEND_H */
