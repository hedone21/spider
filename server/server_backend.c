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

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>
#include "common/log.h"
#include "server_backend.h"
#include "spider_assert.h"

static char* find_backend_path(const char *backend_name) {
    char *path = calloc(1, sizeof(char) * PATH_MAX);
    spider_assert(path != NULL);

    /* 1. find from current directory */
    strncpy(path, backend_name, PATH_MAX);
    if (access(path, F_OK) != -1 ) {
        goto out;
    }

    /* 2. find from ~/.spider/backends directory */
    strncpy(path, "~/.spider/backends", PATH_MAX);
    strncat(path, backend_name, PATH_MAX);
    if (access(path, F_OK) != -1 ) {
        goto out;
    }

    /* 3. find from /usr/lib/spider/backends directory */
    strncpy(path, "/usr/lib/spider/backends", PATH_MAX);
    strncat(path, backend_name, PATH_MAX);
    if (access(path, F_OK) != -1 ) {
        goto out;
    }

    free(path);
    path = NULL;

out:
    return path;
}

static char* get_backend_path(enum spider_server_backend_type type) {
    char *path = NULL;
    switch (type) {
        case WLROOTS_BACKEND:
            path = find_backend_path("spider_wlroots.so");
            break;
        default:
            spider_err("Unsupported backend (type=%d)\n", type);
            break;
    }

    return path;
}

struct spider_server_backend* spider_server_backend_create_with_path(char *backend_path) {
    struct spider_server_backend *backend = NULL;
    void *backend_handle = NULL;

    backend = calloc(1, sizeof(*backend));
    spider_assert(backend != NULL);

    backend_handle = dlopen(backend_path, RTLD_LAZY);
    spider_assert(backend_handle != NULL);
    
    backend->init = dlsym(backend_handle, "spider_backend_init");
    spider_assert(backend->init != NULL);
    backend->run = dlsym(backend_handle, "spider_backend_run");
    spider_assert(backend->run != NULL);
    backend->free = dlsym(backend_handle, "spider_backend_free");
    spider_assert(backend->free != NULL);

    return backend;
}

struct spider_server_backend* spider_server_backend_create(enum spider_server_backend_type type) {
    struct spider_server_backend *backend = NULL;
    char *backend_path = NULL;
    void *backend_handle = NULL;

    spider_assert(type < NUM_OF_BACKEND);

    backend_path = get_backend_path(type);
    spider_assert(backend_path != NULL);

    backend = spider_server_backend_create_with_path(backend_path);
    spider_assert(backend != NULL);

    backend->backend_type = type;

    return backend;
}

void spider_server_backend_free(struct spider_server_backend **backend) {
    (*backend)->free(*backend); 
    free(*backend);
    *backend = NULL;
}
