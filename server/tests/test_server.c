#include <assert.h>
#include <stdbool.h>
#include "server/server.h"
#include "server/backend/backend.h"
#include "server/backend/server.h"

static void test_server_operation() {
    struct spider_server *server = NULL;
    struct spider_backend *backend = NULL;
    struct spider_backend_server *backend_server = NULL;

    // backend = spider_backend_create_with_sopath("../spider_backend_mock.so");
    backend = spider_backend_create_with_sopath("/home/go/Git/spider/build/server/spider_backend_mock.so");
    assert(backend != NULL);

    server = spider_server_create();
    assert(server != NULL);
    spider_server_add_backend(server, backend);

    spider_server_run(server);

    spider_server_free(&server);
}

int main() {
    test_server_operation();
}
