#include <assert.h>
#include <stdbool.h>
#include "server/client.h"
#include "server/client_mngr.h"

static void test_client_mngr_add_client() {
    struct spider_client_mngr *client_mngr = NULL;
    struct spider_client *client = NULL;
    bool ret = 0;

    client_mngr = spider_client_mngr_new();
    assert(client_mngr != NULL);

    /* NULL client */
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret != true);
    spider_client_mngr_free(client_mngr);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret != true);
    spider_client_mngr_free(client_mngr);
    ret = spider_client_mngr_insert_client(client_mngr, client, 0);
    assert(ret != true);
    spider_client_mngr_free(client_mngr);

    /* overlapped pid */
    client = spider_client_create(0);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(0);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret != true);
    spider_client_mngr_free(client_mngr);

    client = spider_client_create(0);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(0);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret != true);
    spider_client_mngr_free(client_mngr);

    client = spider_client_create(0);
    ret = spider_client_mngr_insert_client(client_mngr, client, 0);
    assert(ret == true);
    client = spider_client_create(0);
    ret = spider_client_mngr_insert_client(client_mngr, client, 1);
    assert(ret != true);
    spider_client_mngr_free(client_mngr);

    /* append client with shell, panel */
    client = spider_client_create(0);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(1);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(2);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_shell(3);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_panel(4, 0);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_panel(5, 1);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(6);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    for (int i = 0; i < 7; i++) {
        client = spider_client_mngr_get_client(client_mngr, i);
        assert(client->pid == i);
    }
    spider_client_mngr_free(client_mngr);

    /* prepend client */
    client = spider_client_create(0);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(1);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(2);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_shell(3);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_panel(4, 0);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_panel(5, 1);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(6);
    ret = spider_client_mngr_prepend_client(client_mngr, client);
    assert(ret == true);
    for (int i = 0; i < 7; i++) {
        client = spider_client_mngr_get_client(client_mngr, i);
        assert(client->pid == 6 - i);
    }
    spider_client_mngr_free(client_mngr);

    /* insert client */
    client = spider_client_create(0);
    ret = spider_client_mngr_insert_client(client_mngr, client, 0);
    assert(ret == true);
    client = spider_client_create(1);
    /* just print warning */
    ret = spider_client_mngr_insert_client(client_mngr, client, 2);
    assert(ret == true);
    client = spider_client_create(2);
    ret = spider_client_mngr_insert_client(client_mngr, client, 0);
    assert(ret == true);
    /* pid: 2 -> 0 -> 1 */
    client = spider_client_mngr_get_client(client_mngr, 0);
    assert(client->pid == 2);
    client = spider_client_mngr_get_client(client_mngr, 1);
    assert(client->pid == 0);
    client = spider_client_mngr_get_client(client_mngr, 2);
    assert(client->pid == 1);
    spider_client_mngr_free(client_mngr);
}

static void test_client_mngr_get_client() {
    struct spider_client_mngr *client_mngr = NULL;
    struct spider_client *client = NULL;
    bool ret = 0;

    client_mngr = spider_client_mngr_new();

    /* append client with shell, panel */
    client = spider_client_create(0);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(1);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(2);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_shell(3);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_panel(4, 0);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create_panel(5, 1);
    ret = spider_client_mngr_append_client(client_mngr, client);
    assert(ret == true);
    client = spider_client_create(6);
    ret = spider_client_mngr_append_client(client_mngr, client);

    int cnt = spider_client_mngr_get_client_cnt(client_mngr);
    assert(cnt == 7);

    struct spider_iter *iter = spider_client_mngr_get_client_iter(client_mngr);
    assert(iter != NULL);
    for (; iter == NULL; iter = iter->next(iter)) {
        int pos = iter->pos;
        client = (struct spider_client*)iter->data;
        assert(pos == client->pid);
    }

    client = spider_client_mngr_get_client(client_mngr, 2);
    assert(client->pid == 2);
    client = spider_client_mngr_get_shell(client_mngr);
    assert(client->pid == 3);
    client = spider_client_mngr_get_panel(client_mngr, 0);
    assert(client->pid == 4);
    client = spider_client_mngr_get_panel(client_mngr, 1);
    assert(client->pid == 5);

    client = spider_client_mngr_get_client(client_mngr, 2);
    assert(client->pid == 2);
    spider_client_mngr_remove_client(client_mngr, 2);
    client = spider_client_mngr_get_client(client_mngr, 2);
    assert(client->pid == 3);
    client = spider_client_mngr_get_shell(client_mngr);
    assert(client->pid == 3);
    client = spider_client_mngr_get_panel(client_mngr, 0);
    assert(client->pid == 4);
    client = spider_client_mngr_get_panel(client_mngr, 1);
    assert(client->pid == 5);
    spider_client_mngr_remove_client(client_mngr, 4);
    client = spider_client_mngr_get_panel(client_mngr, 0);
    assert(client == NULL);
    spider_client_mngr_free(client_mngr);
}

int main() {
    test_client_mngr_add_client();
    test_client_mngr_get_client();

    return 0;
}
