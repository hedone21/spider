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

#ifndef SPIDER_CLIENT_MNGR_H
#define SPIDER_CLIENT_MNGR_H

#include <glib.h>
#include <stdbool.h>
#include "client.h"
#include "iter.h"

struct spider_client_mngr {
    GList *clients;
    struct spider_client *shell_client;
    struct spider_client *panel1_client;
    struct spider_client *panel2_client;
    unsigned int client_cnt;
};

struct spider_client_mngr* spider_client_mngr_new();
bool spider_client_mngr_append_client(struct spider_client_mngr *mngr, struct spider_client *client);
bool spider_client_mngr_prepend_client(struct spider_client_mngr *mngr, struct spider_client *client);
bool spider_client_mngr_insert_client(struct spider_client_mngr *mngr, struct spider_client *client, int idx);
struct spider_client* spider_client_mngr_get_client(struct spider_client_mngr *mngr, int idx);
struct spider_client* spider_client_mngr_get_client_with_id(struct spider_client_mngr *mngr, int id);
struct spider_client* spider_client_mngr_get_client_with_obj(struct spider_client_mngr *mngr, struct spider_client *client);
struct spider_client* spider_client_mngr_get_shell(struct spider_client_mngr *mngr);
struct spider_client* spider_client_mngr_get_panel(struct spider_client_mngr *mngr, int idx);
struct spider_iter* spider_client_mngr_get_client_iter(struct spider_client_mngr *mngr);
unsigned int spider_client_mngr_get_client_cnt(struct spider_client_mngr *mngr);
struct spider_client* spider_client_mngr_get_shell(struct spider_client_mngr *mngr);
void spider_client_mngr_focus(struct spider_client_mngr *mngr, unsigned int x, unsigned int y);
void spider_client_mngr_remove_client(struct spider_client_mngr *mngr, int idx);
void spider_client_mngr_remove_client_with_id(struct spider_client_mngr *mngr, int id);
void spider_client_mngr_remove_client_with_obj(struct spider_client_mngr *mngr, struct spider_client *client);
void spider_client_mngr_free(struct spider_client_mngr **mngr);

#endif /* SPIDER_CLIENT_MNGR_H */
