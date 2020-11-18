#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "server/event.h"
#include "server/iter.h"

struct test_obj {
    int test_arg1;
    int test_arg2;
};

static void test_event_register_and_get() {
    struct spider_event *event = NULL;
    int int_data = 1234;
    char *char_data = "4567";
    struct test_obj obj_data = {567, 890};
    struct spider_iter *iter = NULL;
    unsigned int ret_pos = 0;
    int ret_int;
    char *ret_char;
    struct test_obj *ret_obj;

    event = spider_event_create();

    spider_event_register(event, NEW_CLIENT_EVENT, int_data); 
    spider_event_register(event, DEL_CLIENT_EVENT, char_data); 
    spider_event_register(event, FULL_WINDOW_EVENT, char_data); 
    spider_event_register(event, MIN_WINDOW_EVENT, &obj_data); 
    spider_event_register(event, MAX_WINDOW_EVENT, &obj_data); 
    spider_event_register(event, FULL_WINDOW_EVENT, int_data); 

    iter = spider_event_get_iter(event, NEW_CLIENT_EVENT);
    assert(iter != NULL);
    for (; iter != NULL; iter = iter->next(iter)) {
        ret_pos = spider_iter_get_pos(iter);
        ret_int = spider_iter_get_data(iter);

        assert(ret_pos == 0);
        assert(ret_int == 1234);
    }

    iter = spider_event_get_iter(event, DEL_CLIENT_EVENT);
    assert(iter != NULL);
    for (; iter != NULL; iter = iter->next(iter)) {
        ret_pos = spider_iter_get_pos(iter);
        ret_char = spider_iter_get_data(iter);

        assert(ret_pos == 0);
        assert(strcmp(ret_char, "4567") == 0);
    }

    iter = spider_event_get_iter(event, MIN_WINDOW_EVENT);
    assert(iter != NULL);
    for (; iter != NULL; iter = iter->next(iter)) {
        ret_pos = spider_iter_get_pos(iter);
        ret_obj = spider_iter_get_data(iter);

        assert(ret_pos == 0);
        assert(ret_obj->test_arg1 == 567);
        assert(ret_obj->test_arg2 == 890);
    }

    iter = spider_event_get_iter(event, MAX_WINDOW_EVENT);
    assert(iter != NULL);
    for (; iter != NULL; iter = iter->next(iter)) {
        ret_pos = spider_iter_get_pos(iter);
        ret_obj = spider_iter_get_data(iter);

        assert(ret_pos == 0);
        assert(ret_obj->test_arg1 == 567);
        assert(ret_obj->test_arg2 == 890);
    }

    iter = spider_event_get_iter(event, FULL_WINDOW_EVENT);
    assert(iter != NULL);
    for (; iter != NULL; iter = iter->next(iter)) {
        ret_pos = spider_iter_get_pos(iter);

        switch (ret_pos) {
            case 0:
                ret_char = spider_iter_get_data(iter);
                assert(strcmp(ret_char, "4567") == 0);
                break;
            case 1:
                ret_int = spider_iter_get_data(iter);
                assert(ret_int == 1234);
                break;
            default:
                abort();
                break;
        }
    }

    spider_event_free(&event);
}

int main() {
    test_event_register_and_get();
}
