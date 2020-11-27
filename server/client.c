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
#include "client.h"
#include "spider_assert.h"
#include "common/log.h"
#include "server/backend/client.h"

struct spider_client* spider_client_create(int id) {
    struct spider_client *client = NULL;

    client = calloc(1, sizeof(*client));
    if (client == NULL) {
        spider_err("Failed to alloc memory\n");
        return NULL;
    }

    client->id = id;
    client->layer = CLIENT_LAYER;

    return client;
}

struct spider_client* spider_client_create_shell(int id) {
    struct spider_client *client = NULL;

    client = spider_client_create(id);
    client->layer = SHELL_LAYER;

    return client;
}

struct spider_client* spider_client_create_panel(int id, int panel_id) {
    struct spider_client *client = NULL;

    client = spider_client_create(id);
    if (panel_id == 0) {
        client->layer = PANEL1_LAYER;
    }else if (panel_id == 1) {
        client->layer = PANEL2_LAYER;
    }else {
        spider_err("Unsupported panel id\n");
        goto err;
    }

    return client;

err:
    spider_client_free(&client);
    return NULL;
}

void spider_client_add_backend(struct spider_client *client, struct spider_backend *backend) {
    spider_assert(client != NULL);
    spider_assert(backend != NULL);
    if (client->backend != NULL) {
        spider_err("Client backend is already exist\n");
    }

    client->backend = spider_backend_client_get(backend);
    client->backend->new(client, client->data);
}

enum spider_client_layer spider_client_get_layer(struct spider_client *client) {
    spider_assert(client != NULL);
    return client->layer;
}

struct spider_window* spider_client_get_window(struct spider_client *client) {
    spider_assert(client != NULL);
    return &(client->window);
} 

void spider_client_draw(struct spider_client *client) {
    spider_assert(client != NULL);

    if (client->backend) {
        client->backend->draw(client, client->data);
    }
}

void spider_client_free(struct spider_client **client) {
    if (*client == NULL) {
        return;
    }

    if ((*client)->backend) {
        (*client)->backend->free(*client, (*client)->data);
    }

    free(*client);
    *client = NULL;
}
