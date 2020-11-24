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

#include <glib.h>
#include <stdlib.h>
#include "spider_assert.h"
#include "client_mngr.h"
#include "client.h"
#include "common/log.h"
#include "iter.h"

static struct spider_iter* create_new_iter(struct spider_client_mngr *mngr);
static struct spider_iter* get_iter_next(struct spider_iter *iter);

static struct spider_iter* create_new_iter(struct spider_client_mngr *mngr) {
    struct spider_iter *iter = NULL;
    void *data = spider_client_mngr_get_client(mngr, 0);
    if (data == NULL) {
        return NULL;
    }

    iter = calloc(1, sizeof(*iter));
    spider_assert(iter);

    iter->_user_data = mngr;
    iter->data = spider_client_mngr_get_client(mngr, 0);
    iter->next = get_iter_next;

    return iter;
}

static struct spider_iter* get_iter_next(struct spider_iter *iter) {
    struct spider_iter *ret_iter = create_new_iter(iter->_user_data);
    spider_assert(ret_iter);

    struct spider_client_mngr *mngr = ret_iter->_user_data;

    ret_iter->pos = iter->pos + 1;
    if (ret_iter->pos >= spider_client_mngr_get_client_cnt(mngr)) {
        free(ret_iter);
        ret_iter = NULL;
        goto out;
    }

    ret_iter->data = spider_client_mngr_get_client(mngr, ret_iter->pos);
    spider_assert(ret_iter->data);

out:
    free(iter);

    return ret_iter;
}

struct spider_client_mngr* spider_client_mngr_new() {
    struct spider_client_mngr *mngr = NULL;

    mngr = calloc(1, sizeof(*mngr));
    spider_assert(mngr);

    return mngr;
}

static bool check_client_already_exist(struct spider_client_mngr *mngr, struct spider_client *client) {
    spider_assert(mngr);
    spider_assert(client);

    struct spider_client *exist = NULL;

    exist = spider_client_mngr_get_client_with_obj(mngr, client);
    if (exist) {
        spider_err("Client object already exist\n");
        return false;
    }

    exist = spider_client_mngr_get_client_with_id(mngr, client->id);
    if (exist) {
        spider_err("Client id already exist\n");
        return false;
    }

    return true;
}

static bool save_client_to_special_layer(struct spider_client_mngr *mngr, struct spider_client *client) {
    spider_assert(mngr);
    spider_assert(client);

    if (client->layer == SHELL_LAYER) {
        if (mngr->shell_client != NULL) {
            spider_err("Shell client object already exist\n");
            return false;
        }
        mngr->shell_client = client;
    }else if (client->layer == PANEL1_LAYER) {
        if (mngr->panel1_client != NULL) {
            spider_err("Panel 1 client object already exist\n");
            return false;
        }
        mngr->panel1_client = client;
    }else if (client->layer == PANEL2_LAYER) {
        if (mngr->panel2_client != NULL) {
            spider_err("Panel 2 client object already exist\n");
            return false;
        }
        mngr->panel2_client = client;
    }

    return true;
}

bool spider_client_mngr_append_client(struct spider_client_mngr *mngr, struct spider_client *client) {
    spider_assert(mngr);

    return spider_client_mngr_insert_client(mngr, client, -1);
}

bool spider_client_mngr_prepend_client(struct spider_client_mngr *mngr, struct spider_client *client) {
    spider_assert(mngr);
    bool ret;

    if (client == NULL) {
        spider_err("Null client object\n");
        return false;
    }
 
    ret = check_client_already_exist(mngr, client);
    if (ret == false) {
        return false;
    }

    ret = save_client_to_special_layer(mngr, client);
    if (ret == false) {
        return false;
    }

    mngr->clients = g_list_prepend(mngr->clients, client);
    mngr->client_cnt += 1;
    
    return true;
}

bool spider_client_mngr_insert_client(struct spider_client_mngr *mngr, struct spider_client *client, int idx) {
    spider_assert(mngr);
    bool ret;

    if (client == NULL) {
        spider_err("Null client object\n");
        return false;
    }
 
    ret = check_client_already_exist(mngr, client);
    if (ret == false) {
        return false;
    }

    ret = save_client_to_special_layer(mngr, client);
    if (ret == false) {
        return false;
    }

    mngr->clients = g_list_insert(mngr->clients, client, idx);
    mngr->client_cnt += 1;
    
    return true;
}

struct spider_client* spider_client_mngr_get_client(struct spider_client_mngr *mngr, int idx) {
    spider_assert(mngr);
    return g_list_nth_data(mngr->clients, idx);
}

struct spider_client* spider_client_mngr_get_client_with_id(struct spider_client_mngr *mngr, pid_t id) {
    spider_assert(mngr);
    struct spider_iter *iter = NULL;
    struct spider_client *client = NULL;

    iter = spider_client_mngr_get_client_iter(mngr);
    for (; iter != NULL; iter = iter->next(iter)) {
        client = spider_iter_get_data(iter);
        if (client == NULL) {
            break;
        }

        if (client->id == id) {
            break;
        }

        client = NULL;
    }

    return client;
}

struct spider_client* spider_client_mngr_get_client_with_obj(struct spider_client_mngr *mngr, 
                                                             struct spider_client *client) {
    spider_assert(mngr);
    spider_assert(client);
    struct spider_iter *iter = NULL;
    struct spider_client *client2 = NULL;

    iter = spider_client_mngr_get_client_iter(mngr);
    for (; iter != NULL; iter = iter->next(iter)) {
        client2 = spider_iter_get_data(iter);
        if (client2 == NULL) {
            break;
        }

        if (client2 == client) {
            break;
        }

        client2 = NULL;
    }

    return client2;
}

struct spider_client* spider_client_mngr_get_shell(struct spider_client_mngr *mngr) {
    spider_assert(mngr);
    return mngr->shell_client;
}

struct spider_client* spider_client_mngr_get_panel(struct spider_client_mngr *mngr, int idx) {
    spider_assert(mngr);
    return (idx == 0) ? mngr->panel1_client : mngr->panel2_client;
}

struct spider_iter* spider_client_mngr_get_client_iter(struct spider_client_mngr *mngr) {
    spider_assert(mngr);
    struct spider_iter *iter = create_new_iter(mngr);
    return iter;
}

unsigned int spider_client_mngr_get_client_cnt(struct spider_client_mngr *mngr) {
    return mngr->client_cnt;
}

static void remove_client(struct spider_client_mngr *mngr, struct spider_client *client) {
    spider_assert(mngr);
    spider_assert(client);

    mngr->clients = g_list_remove(mngr->clients, client);
    if (mngr->shell_client == client) {
        mngr->shell_client = NULL;
    }else if (mngr->panel1_client == client) {
        mngr->panel1_client = NULL;
    }else if (mngr->panel2_client == client) {
        mngr->panel2_client = NULL;
    }
    spider_client_free(&client);
    mngr->client_cnt -= 1;
}

void spider_client_mngr_remove_client(struct spider_client_mngr *mngr, int idx) {
    spider_assert(mngr);
    struct spider_client *client = spider_client_mngr_get_client(mngr, idx);
    if (client == NULL) {
        spider_err("Cannot remove NULL client (idx=%d)\n", idx);
        return;
    }

    remove_client(mngr, client);
}

void spider_client_mngr_remove_client_with_id(struct spider_client_mngr *mngr, int id) {
    spider_assert(mngr);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, id);
    if (client == NULL) {
        spider_err("Cannot remove NULL client (id=%d)\n", id);
        return;
    }

    remove_client(mngr, client);
}

void spider_client_mngr_remove_client_with_obj(struct spider_client_mngr *mngr, struct spider_client *client) {
    spider_assert(mngr);
    client = spider_client_mngr_get_client_with_obj(mngr, client);
    if (client == NULL) {
        spider_err("Cannot remove NULL client (obj=%p)\n", client);
        return;
    }

    remove_client(mngr, client);
}

void spider_client_mngr_free(struct spider_client_mngr **mngr) {
    spider_assert(*mngr);
    struct spider_iter *iter = NULL;
    struct spider_client *client = NULL;

    iter = spider_client_mngr_get_client_iter(*mngr);
    for (; iter != NULL; iter = iter->next(iter)) {
        client = spider_iter_get_data(iter);
        if (client == NULL) {
            break;
        }

        spider_client_free(&client);
    }

    g_list_free((*mngr)->clients);
    free(iter);
    free(*mngr);
    *mngr = NULL;
}
