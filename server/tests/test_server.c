#include <assert.h>
#include <stdbool.h>
#include "server/server.h"
#include "server/server_backend.h"

static void test_server_operation() {
    struct spider_server *server = NULL;
    server = spider_server_create_with_backendpath("/home/go/Git/spider/build/server/spider_wlroots.so");
    assert(server != NULL);

    spider_server_free(&server);
}

int main() {
    test_server_operation();
}
