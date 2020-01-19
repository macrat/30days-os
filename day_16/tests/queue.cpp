#include <CppUTest/TestHarness.h>

#include "../queue.h"


static queue_t* queue;


TEST_GROUP(Queue) {
    void setup() {
        queue = create_queue(10, sizeof(int));
    }
    void teardown() {
        free(queue);
    }
};


TEST(Queue, ItemCount) {
    LONGS_EQUAL(0, queue_item_count(queue));

    int val = 10;
    queue_push(queue, &val);
    LONGS_EQUAL(1, queue_item_count(queue));

    val = 20;
    queue_push(queue, &val);
    LONGS_EQUAL(2, queue_item_count(queue));

    queue_pop(queue, nullptr);
    LONGS_EQUAL(1, queue_item_count(queue));

    queue_pop(queue, nullptr);
    LONGS_EQUAL(0, queue_item_count(queue));
}


TEST(Queue, Push_Pop) {
    const int data[] = {10, 20, 30, 42, 84, -1, -100, 0x10ff80ff};

    for (int i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
        queue_push(queue, &data[i]);
    }

    for (int i = 0; i < sizeof(data)/sizeof(data[0]); i++) {
        int val;
        queue_pop(queue, &val);
        LONGS_EQUAL(data[i], val);
    }
}


TEST(Queue, PushAndPop) {
    int val = 10;
    queue_push(queue, &val);
    val = 20;
    queue_push(queue, &val);

    LONGS_EQUAL(2, queue_item_count(queue));

    queue_pop_and_push(queue, &val);
    LONGS_EQUAL(10, val);
    LONGS_EQUAL(2, queue_item_count(queue));

    queue_pop_and_push(queue, &val);
    LONGS_EQUAL(20, val);
    LONGS_EQUAL(2, queue_item_count(queue));

    queue_pop_and_push(queue, &val);
    LONGS_EQUAL(10, val);
    LONGS_EQUAL(2, queue_item_count(queue));
}


TEST(Queue, DropByData) {
    const int input[] = {10, 20, 30, 42};
    for (int i = 0; i < sizeof(input)/sizeof(input[0]); i++) {
        queue_push(queue, &input[i]);
    }

    auto assert_queue = [](const int expect[]){
        int i = 0;
        for (queue_item_t* itr = queue->first; itr; itr = itr->next) {
            LONGS_EQUAL(expect[i], *(int*)itr->payload);
            i++;
        }
    };

    assert_queue(input);
    LONGS_EQUAL(4, queue_item_count(queue));

    int val = 30;
    queue_drop_by_data(queue, &val);
    const int drop30[] = {10, 20, 42};
    assert_queue(drop30);
    LONGS_EQUAL(3, queue_item_count(queue));

    val = 10;
    queue_drop_by_data(queue, &val);
    const int drop10[] = {20, 42};
    assert_queue(drop10);
    LONGS_EQUAL(2, queue_item_count(queue));

    val = 42;
    queue_drop_by_data(queue, &val);
    const int drop42[] = {20};
    assert_queue(drop42);
    LONGS_EQUAL(1, queue_item_count(queue));

    val = 84;
    queue_push(queue, &val);
    const int append84[] = {20, 84};
    assert_queue(append84);
    LONGS_EQUAL(2, queue_item_count(queue));
}
