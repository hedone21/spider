#include <assert.h>
#include <stdbool.h>
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
    assert(server != NULL);
    struct spider_client *client = spider_client_create(id++);
    assert(client != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    spider_client_mngr_append_client(mngr, client);
}

static bool del_client_cb(struct spider_server *server, int client_id) {
    assert(server != NULL);
    struct spider_client_mngr *mngr = spider_server_get_client_mngr(server);
    assert(mngr != NULL);
    spider_client_mngr_remove_client_with_id(mngr, client_id);
}

static bool new_window_cb(struct spider_server *server, int client_id) {

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

    spider_server_add_backend(server, backend);

    spider_server_run(server);

    spider_server_free(&server);
}

int main() {
    test_server_operation();
    test_server_operation_with_backend();
    test_server_operation_with_callbacks();
}
