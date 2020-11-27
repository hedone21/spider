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

#include <unistd.h>
#include "server.h"
#include "server/event.h"
#include "server/server.h"
#include "common/log.h"

static bool spider_backend_mock_init(struct spider_server *server, void *data) {
    spider_log("mock init\n");

    return true;
}

static void spider_backend_mock_run(struct spider_server *server, void *data) {
    spider_log("mock run\n");
    const unsigned int max_loop_cnt = 200;
    const unsigned int new_client1_cnt = 10;
    const unsigned int new_client2_cnt = 20;
    const unsigned int new_client3_cnt = 30;
    const unsigned int del_client1_cnt = 170;
    const unsigned int del_client2_cnt = 190;
    const unsigned int del_client3_cnt = 180;

    const unsigned int max_clients = 10;
    int client_id[max_clients]; 

    unsigned int loop_cnt = 0;
    /* 16ms * 200 = 3,200ms */
    for (loop_cnt = 0; loop_cnt < max_loop_cnt; loop_cnt++) {
        if (loop_cnt == new_client1_cnt) {
            spider_server_emit_event(server, NEW_CLIENT_EVENT, &client_id[0]);
        }

        if (loop_cnt == new_client2_cnt) {
            spider_server_emit_event(server, NEW_CLIENT_EVENT, &client_id[1]);
        }

        if (loop_cnt == new_client3_cnt) {
            spider_server_emit_event(server, NEW_CLIENT_EVENT, &client_id[2]);
        }

        if (loop_cnt == del_client1_cnt) {
            spider_server_emit_event(server, DEL_CLIENT_EVENT, client_id[0]);
        }

        if (loop_cnt == del_client2_cnt) {
            spider_server_emit_event(server, DEL_CLIENT_EVENT, client_id[1]);
        }

        if (loop_cnt == del_client3_cnt) {
            spider_server_emit_event(server, DEL_CLIENT_EVENT, client_id[2]);
        }

        usleep(16 * 1000);
    }
}

static void spider_backend_mock_free(struct spider_server *server, void *data) {
    spider_log("mock free\n");

}

extern struct spider_backend_server spider_backend_server = {
    .init = spider_backend_mock_init,
    .run = spider_backend_mock_run,
    .free = spider_backend_mock_free,
    .data = NULL,
};
