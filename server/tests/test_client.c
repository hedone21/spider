#include <assert.h>
#include "server/client.h"

static void test_client_get_layer() {
    enum spider_client_layer layer;
    struct spider_client *client = NULL;

    client = spider_client_create(0);
    layer = spider_client_get_layer(client);
    assert(layer == CLIENT_LAYER);
    spider_client_free(&client);
    
    client = spider_client_create_shell(0);
    layer = spider_client_get_layer(client);
    assert(layer == SHELL_LAYER);
    spider_client_free(&client);
    
    client = spider_client_create_panel(0, 0);
    layer = spider_client_get_layer(client);
    assert(layer == PANEL1_LAYER);
    spider_client_free(&client);
    
    client = spider_client_create_panel(0, 1);
    layer = spider_client_get_layer(client);
    assert(layer == PANEL2_LAYER);
    spider_client_free(&client);

    client = spider_client_create_panel(0, 2);
    assert(client == NULL);
    spider_client_free(&client);

    client = spider_client_create_panel(0, -1);
    assert(client == NULL);
    spider_client_free(&client);
}

static void test_client_free() {
    struct spider_client *client = NULL;
    client = spider_client_create(0);
    assert(client != NULL);
    spider_client_free(&client);
    assert(client == NULL);
}

int main() {
    test_client_get_layer();    
    test_client_free();    

    return 0;
}
