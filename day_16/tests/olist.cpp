#include <CppUTest/TestHarness.h>

#include "../olist.h"


static olist_t* olist;


TEST_GROUP(OList){
    void setup() {
        olist = create_olist(10, sizeof(int));
    }
    void teardown() {
        free(olist);
    }
};


TEST(OList, ItemCount) {
    LONGS_EQUAL(0, olist_item_count(olist));

    int val = 10;
    olist_push(olist, 10, &val);
    LONGS_EQUAL(1, olist_item_count(olist));

    val = 20;
    olist_push(olist, 20, &val);
    LONGS_EQUAL(2, olist_item_count(olist));

    olist_pop(olist, nullptr, nullptr);
    LONGS_EQUAL(1, olist_item_count(olist));

    olist_pop(olist, nullptr, nullptr);
    LONGS_EQUAL(0, olist_item_count(olist));
}


TEST(OList, SortOrder) {
    const int orders[] = {9, 0, 8, 1, 7, 2, 6, 3, 5, 4};
    const int values[] = {1, 3, 5, 7, 9, 8, 6, 4, 2, 0};

    for (int i=0; i<10; i++) {
        olist_push(olist, orders[i], &i);
    }

    int i = 0;
    for (olist_item_t* itr = olist->first; itr; itr = itr->next) {
        LONGS_EQUAL(i, itr->order);
        LONGS_EQUAL(values[i], *(int*)itr->payload);
        i++;
    }

    i = 0;
    int val;
    uint_fast32_t order;
    while (olist_pop(olist, &val, &order)) {
        LONGS_EQUAL(i, order);
        LONGS_EQUAL(values[i], val);
        i++;
    }
}


TEST(OList, DropItem) {
    const int orders[] = {3, 1, 2, 0};
    const int values[] = {10, 20, 30, 40};
    olist_item_t* items[4];

    auto assert_list = [](const int expect[]){
        int i = 0;
        for (olist_item_t* itr = olist->first; itr; itr = itr->next) {
            LONGS_EQUAL(expect[i], *(int*)itr->payload);
            i++;
        }
    };

    for (int i=0; i<4; i++) {
        items[i] = olist_push(olist, orders[i], &values[i]);
    }

    const int initial[] = {40, 20, 30, 10};
    assert_list(initial);

    olist_drop(olist, items[1]);
    const int drop1[] = {40, 30, 10};
    assert_list(drop1);

    olist_drop(olist, items[3]);
    const int drop2[] = {30, 10};
    assert_list(drop2);
}
