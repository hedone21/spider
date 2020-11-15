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

#ifndef SPIDER_CLIENT_H
#define SPIDER_CLIENT_H

#include <unistd.h>
#include "window.h"

enum spider_client_layer {
    SHELL_LAYER = 0, /* == BACKGROUND */
    PANEL1_LAYER,
    PANEL2_LAYER,
    CLIENT_LAYER,
    OVERLAY_LAYER,
};

struct spider_client {
    struct spider_window window;
    enum spider_client_layer layer;
    pid_t pid;
};

struct spider_client* spider_client_create(pid_t pid);
struct spider_client* spider_client_create_shell(pid_t pid);
struct spider_client* spider_client_create_panel(pid_t pid, int panel_id);
enum spider_client_layer spider_client_get_layer(struct spider_client *client);
void spider_client_free(struct spider_client **client);

#endif /* SPIDER_CLIENT_H */