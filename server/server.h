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

#ifndef SPIDER_SERVER_H
#define SPIDER_SERVER_H

#include <stdarg.h>
#include <stdbool.h>
#include <glib.h>
#include "client_mngr.h"
#include "cursor.h"
#include "event.h"
#include "backend/backend.h"

struct spider_server {
    struct spider_client_mngr *mngr; 
    struct spider_cursor *cursor;
    struct spider_backend *backend;
    GList* events[NUM_OF_EVENT];
};

struct spider_server* spider_server_create();
struct spider_server* spider_server_create_with_backend(char *backend);
void spider_server_run(struct spider_server *server);
void spider_server_free(struct spider_server **server);
struct spider_client_mngr* spider_server_get_client_mngr(struct spider_server *server);
struct spider_cursor* spider_server_get_cursor(struct spider_server *server);
bool spider_server_register_event(struct spider_server *server, enum spider_event_type ev_type, void *cb);
void spider_server_emit_event(struct spider_server *server, enum spider_event_type ev_type, ...);

/* This callbacks are corresponed to spider_event_type */

/**
 * @brief Called when a new client is created. MUST be register only one callback. If not, it could be return wrong value
 * @param server spider server object
 * @param client_id client id returned from callback
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_new_client_cb)(struct spider_server *server, int *client_id);

/**
 * @brief Called when a new client is deleted
 * @param server spider server object
 * @param client_id client id to delete
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_del_client_cb)(struct spider_server *server, int client_id);

/**
 * @brief Called when a new client is created
 * @param server spider server object
 * @param client_id client id which own window
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_new_window_cb)(struct spider_server *server, int client_id);

/**
 * @brief Called when a client is maximized
 * @param server spider server object
 * @param client_id client id which own window
 * @param maximized true when window is maximized, and vise versa
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_max_window_cb)(struct spider_server *server, int client_id, bool maximized);

/**
 * @brief Called when a client is minimized
 * @param server spider server object
 * @param client_id client id which own window
 * @param minimized true when window is minimized, and vise versa
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_min_window_cb)(struct spider_server *server, int client_id, bool minimized);

/**
 * @brief Called when a client become a full screen
 * @param server spider server object
 * @param client_id client id which own window
 * @param full true when window is full screen, and vise versa
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_full_window_cb)(struct spider_server *server, int client_id, bool full);

/**
 * @brief Called when a client is moved
 * @param server spider server object
 * @param client_id client id which own window
 * @param x x positional value
 * @param y y positional value
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_move_window_cb)(struct spider_server *server, int client_id, unsigned int x, unsigned int y);

/**
 * @brief Called when a client is resized
 * @param server spider server object
 * @param client_id client id which own window
 * @param w width
 * @param h height
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_resize_window_cb)(struct spider_server *server, int client_id, unsigned int w, unsigned int h);

/**
 * @brief Called when a client is deleted
 * @param server spider server object
 * @param client_id client id which own window
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_del_window_cb)(struct spider_server *server, int client_id);

/**
 * @brief Called when a cursor moved
 * @param server spider server object
 * @param x x relative value
 * @param y y relative value
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_move_cursor_cb)(struct spider_server *server, unsigned int x, unsigned int y);

/**
 * @brief Called when a cursor moved
 * @param server spider server object
 * @param x x absolute value
 * @param y y absolute value
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_absmove_cursor_cb)(struct spider_server *server, unsigned int x, unsigned int y);

/**
 * @brief Called when a client is clicked
 * @param server spider server object
 * @param is_clicked true on pressed, false on released
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_click_cursor_cb)(struct spider_server *server, bool is_clicked);

/**
 * @brief Called when frame is drawed
 * @param server spider server object
 * @return boolean Retern true on success, and return false on failure
 */
typedef bool (*spider_render_cb)(struct spider_server *server);

#endif /* SPIDER_SERVER_H */
