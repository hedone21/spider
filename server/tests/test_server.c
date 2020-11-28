#include <assert.h>
#include <stdbool.h>
#include "common/log.h"
#include "server/server.h"
#include "server/backend/backend.h"
#include "server/backend/server.h"

static void test_server_operation() {
    struct spider_server *server = NULL;
    struct spider_backend_server *backend_server = NULL;

    server = spider_server_create();
    assert(server != NULL);

    spider_server_run(server);

    spider_server_free(&server);
}

static void test_server_operation_with_backend() {
    struct spider_server *server = NULL;
    struct spider_backend *backend = NULL;
    struct spider_backend_server *backend_server = NULL;

    backend = spider_backend_create_with_sopath("server/spider_backend_mock.so");
    assert(backend != NULL);

    server = spider_server_create();
    assert(server != NULL);

    spider_server_add_backend(server, backend);

    spider_server_run(server);

    spider_server_free(&server);
}

static bool new_client_cb(struct spider_server *server, int *client_id) {
    static int id = 0;
    spider_log("new client id=%d\n", id);

    assert(server != NULL);
    struct spider_client *client = spider_client_create(id);
    assert(client != NULL);
    if (server->backend) {
        spider_client_add_backend(client, server->backend);
    }
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    spider_client_mngr_append_client(mngr, client);
    *client_id = id;
    id++;
}

static bool del_client_cb(struct spider_server *server, int client_id) {
    spider_log("del client id=%d\n", client_id);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    spider_client_mngr_remove_client_with_id(mngr, client_id);
}

static bool new_window_cb(struct spider_server *server, int client_id, 
                          unsigned int x, unsigned int y, unsigned int w, unsigned int h) {
    spider_log("new window id=%d\n", client_id);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, client_id);
    assert(client != NULL);
    struct spider_window *window = spider_client_get_window(client);
    assert(window != NULL);
    window->x = x;
    window->y = y;
    window->w = w;
    window->h = h;
}

static bool max_window_cb(struct spider_server *server, int client_id, bool maximized) {
    spider_log("max window id=%d maximized=%b\n", client_id, maximized);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, client_id);
    assert(client != NULL);
    struct spider_window *window = spider_client_get_window(client);
    assert(window != NULL);
    spider_window_maximize(window, maximized);
    if (maximized) {
        assert(spider_window_is_maximized(window) == maximized);
        assert(spider_window_is_minimized(window) != maximized);
        assert(spider_window_is_fullscreen(window) != maximized);
    }
}

static bool min_window_cb(struct spider_server *server, int client_id, bool minimized) {
    spider_log("min window id=%d minimized=%d\n", client_id, minimized);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, client_id);
    assert(client != NULL);
    struct spider_window *window = spider_client_get_window(client);
    assert(window != NULL);
    spider_window_minimize(window, minimized);
    if (minimized) {
        assert(spider_window_is_maximized(window) != minimized);
        assert(spider_window_is_minimized(window) == minimized);
        assert(spider_window_is_fullscreen(window) != minimized);
    }
}

static bool full_window_cb(struct spider_server *server, int client_id, bool is_full) {
    spider_log("full window id=%d is_full=%b\n", client_id, is_full);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, client_id);
    assert(client != NULL);
    struct spider_window *window = spider_client_get_window(client);
    assert(window != NULL);
    spider_window_full(window, is_full);
    if (is_full) {
        assert(spider_window_is_maximized(window) != is_full);
        assert(spider_window_is_minimized(window) != is_full);
        assert(spider_window_is_fullscreen(window) == is_full);
    }
}

static bool move_window_cb(struct spider_server *server, int client_id, unsigned int x, unsigned int y) {
    spider_log("move window id=%d x=%d y=%d\n", client_id, x, y);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, client_id);
    assert(client != NULL);
    struct spider_window *window = spider_client_get_window(client);
    assert(window != NULL);
    spider_window_move(window, x, y);
    assert(window->x == x);
    assert(window->y == y);
}

static bool resize_window_cb(struct spider_server *server, int client_id, unsigned int w, unsigned int h) {
    spider_log("resize window id=%d w=%d h=%d\n", client_id, w, h);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, client_id);
    assert(client != NULL);
    struct spider_window *window = spider_client_get_window(client);
    assert(window != NULL);
    spider_window_resize(window, w, h);
    assert(window->w == w);
    assert(window->h == h);
}

static bool del_window_cb(struct spider_server *server, int client_id) {
    spider_log("del window id=%d\n", client_id);

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_client *client = spider_client_mngr_get_client_with_id(mngr, client_id);
    assert(client != NULL);
    struct spider_window *window = spider_client_get_window(client);
    assert(window != NULL);
}

static bool absmove_cursor_cb(struct spider_server *server, unsigned int x, unsigned int y) {
    spider_log("absmove cursor\n");
    
    assert(server != NULL);
    struct spider_cursor *cursor = spider_server_get_cursor(server);
    assert(cursor != NULL);

    spider_cursor_absmove(cursor, x, y);
}

static bool click_cursor_cb(struct spider_server *server, bool is_clicked) {
    spider_log("click cursor\n");

    assert(server != NULL);
    struct spider_cursor *cursor = spider_server_get_cursor(server);
    assert(cursor != NULL);

    spider_cursor_click(cursor, is_clicked);
    if (is_clicked) {
        int x, y;
        spider_cursor_get_pos(cursor, &x, &y);
        spider_client_mngr_focus(cursor, x, y);
    }
}

static bool render_cb(struct spider_server *server) {
    spider_log("render\n");

    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    struct spider_iter *iter = spider_client_mngr_get_client_iter(mngr);
    for (; iter != NULL; iter = iter->next(iter)) {
        struct spider_client *client = spider_iter_get_data(iter);
        spider_client_draw(client);
    }
}

static void test_server_operation_with_callbacks() {
    struct spider_server *server = NULL;
    struct spider_backend *backend = NULL;
    struct spider_backend_server *backend_server = NULL;

    backend = spider_backend_create_with_sopath("server/spider_backend_mock.so");
    assert(backend != NULL);

    server = spider_server_create();
    assert(server != NULL);

    spider_server_register_event(server, NEW_CLIENT_EVENT, new_client_cb);
    spider_server_register_event(server, DEL_CLIENT_EVENT, del_client_cb);
    spider_server_register_event(server, NEW_WINDOW_EVENT, new_window_cb);
    spider_server_register_event(server, MAX_WINDOW_EVENT, max_window_cb);
    spider_server_register_event(server, MIN_WINDOW_EVENT, min_window_cb);
    spider_server_register_event(server, FULL_WINDOW_EVENT, full_window_cb);
    spider_server_register_event(server, MOVE_WINDOW_EVENT, move_window_cb);
    spider_server_register_event(server, RESIZE_WINDOW_EVENT, resize_window_cb);
    spider_server_register_event(server, DEL_WINDOW_EVENT, del_window_cb);
    spider_server_register_event(server, ABSMOVE_CURSOR_EVENT, absmove_cursor_cb);
    spider_server_register_event(server, CLICK_CURSOR_EVENT, click_cursor_cb);
    spider_server_register_event(server, RENDER_EVENT, render_cb);

    spider_server_add_backend(server, backend);

    spider_server_run(server);

    spider_server_free(&server);
}

int main() {
    test_server_operation();
    test_server_operation_with_backend();
    test_server_operation_with_callbacks();
}
