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

#include "spider_assert.h"
#include "window.h"

void spider_window_move(struct spider_window *window, unsigned int x, unsigned int y) {
    spider_assert(window);

    window->x = x;
    window->y = y;
}

void spider_window_resize(struct spider_window *window, unsigned int w, unsigned int h) {
    spider_assert(window);

    window->w = w;
    window->h = h;
}

void spider_window_maximize(struct spider_window *window, bool is_maximized) {
    spider_assert(window);

    window->is_maximized = is_maximized;
    if (is_maximized) {
        window->is_minimized = false;
        window->is_full = false;
    }
}

void spider_window_minimize(struct spider_window *window, bool is_minimized) {
    spider_assert(window);

    window->is_minimized = is_minimized;
    if (is_minimized) {
        window->is_maximized = false;
        window->is_full = false;
    }
}

void spider_window_full(struct spider_window *window, bool is_full) {
    spider_assert(window);

    window->is_full = is_full;
    if (is_full) {
        window->is_maximized = false;
        window->is_minimized = false;
    }
}

bool spider_window_is_maximized(struct spider_window *window) {
    spider_assert(window);

    return window->is_maximized;
}

bool spider_window_is_minimized(struct spider_window *window) {
    spider_assert(window);

    return window->is_minimized;
}

bool spider_window_is_fullscreen(struct spider_window *window) {
    spider_assert(window);

    return window->is_full;
}
