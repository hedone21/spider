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
#include "backend.h"
#include "server/spider_assert.h"

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

struct spider_backend* spider_backend_create(enum spider_backend_type type) {
    spider_assert(type < NUM_OF_BACKEND);

    struct spider_backend *backend = NULL;
    char *backend_path = NULL;

    backend_path = spider_backend_get_path(type);
    spider_assert(backend_path != NULL);

    backend = spider_backend_create_with_sopath(backend_path);
    spider_assert(backend != NULL);

    backend->type = type;

    return backend;
}

struct spider_backend* spider_backend_create_with_sopath(const char *path) {
    struct spider_backend *backend = NULL;
    void *backend_handle = NULL;

    backend_handle = dlopen(path, RTLD_LAZY);
    if (backend_handle == NULL) {
        spider_err("Failed to get backend so file (%s)\n", path);
        return NULL;
    }

    backend = calloc(1, sizeof(*backend));
    spider_assert(backend != NULL);

    backend->handle = backend_handle;

    return backend;
}

void* spider_backend_get_sym(struct spider_backend *backend, const char *sym) {
    spider_assert(backend != NULL);
    void *ret = NULL;

    ret = dlsym(backend->handle, sym);
    spider_assert(ret != NULL);

    return ret;
}

char* spider_backend_get_path(enum spider_backend_type type) {
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

void spider_backend_free(struct spider_backend **backend) {
    if (*backend == NULL) {
        return;
    }

    dlclose((*backend)->handle);
}
