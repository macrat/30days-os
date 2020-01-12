#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct OListItem {
    bool              active;
    uint_fast32_t     order;
    struct OListItem* next;
    uint8_t           payload[];
} olist_item_t;


typedef struct OList {
    size_t       pool_size, payload_size;
    olist_item_t *first;
    olist_item_t pool[];
} olist_t;


extern olist_t* create_olist(size_t pool_size, size_t payload_size);

extern olist_item_t* olist_push(olist_t* olist, uint_fast32_t order, const void* payload);
extern void* olist_pop(olist_t* olist, void* payload_buf, uint_fast32_t* order_buf);
extern void olist_drop(olist_t* olist, olist_item_t* item);

extern void olist_reorder(olist_t* olist, olist_item_t* item, uint_fast32_t new_order);

extern uint_fast32_t olist_item_count(const olist_t* olist);
