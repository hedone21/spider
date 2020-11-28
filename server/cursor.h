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

#ifndef SPIDER_CURSOR_H
#define SPIDER_CURSOR_H

#include <stdbool.h>

struct spider_cursor {
    unsigned int x, y;
    bool is_clicked;
};

struct spider_cursor* spider_cursor_new();
void spider_cursor_move(struct spider_cursor *cursor, unsigned int x, unsigned int y);
void spider_cursor_absmove(struct spider_cursor *cursor, unsigned int x, unsigned int y);
void spider_cursor_click(struct spider_cursor *cursor, bool is_clicked);
void spider_cursor_get_pos(struct spider_cursor *cursor, unsigned int *x, unsigned int *y);
bool spider_cursor_is_clicked(struct spider_cursor *cursor);
void spider_cursor_free(struct spider_cursor **cursor);

// void spider_cursor_axis(struct spider_cursor *cursor, struct spider_server *server);

#endif /* SPIDER_CURSOR_H */
