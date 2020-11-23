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

static void test_server_callback(struct spider_server *server, void *data) {
}

static void test_server_operation_with_callbacks() {
    struct spider_server *server = NULL;
    struct spider_backend *backend = NULL;
    struct spider_backend_server *backend_server = NULL;

    backend = spider_backend_create_with_sopath("server/spider_backend_mock.so");
    assert(backend != NULL);

    /* Types of callbacks
     * [Server]
     * - START_SERVER (struct spider_server *server)
     * - STOP_SERVER (struct spider_server *server)
     * 
     * [Client]
     * - NEW_CLIENT (struct spider_server *server, struct spider_client *client)
     * - DEL_CLIENT (struct spider_server *server, struct spider_client *client))
     *
     * [Window]
     * - NEW_WINDOW (struct spider_server *server, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
     */

    server = spider_server_create();
    assert(server != NULL);

    spider_server_add_backend(server, backend);

    spider_server_run(server);

    spider_server_free(&server);
}

int main() {
    test_server_operation();
    test_server_operation_with_backend();
    test_server_operation_with_callbacks();
}
