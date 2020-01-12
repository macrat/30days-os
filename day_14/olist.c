#include "memory.h"
#include "olist.h"


size_t olist_sizeof(size_t pool_size, size_t payload_size) {
    return offsetof(olist_t, pool) + (offsetof(olist_item_t, payload) + payload_size) * pool_size;
}

olist_t* create_olist_on(void* dest, size_t pool_size, size_t payload_size) {
    memset(dest, 0x00, olist_sizeof(pool_size, payload_size));

    olist_t* olist = (olist_t*)dest;

    olist->pool_size = pool_size;
    olist->payload_size = payload_size;

    return olist;
}

olist_t* create_olist(size_t pool_size, size_t payload_size) {
    void* buf = malloc(olist_sizeof(pool_size, payload_size));
    if (buf == 0) return 0;

    return create_olist_on(buf, pool_size, payload_size);
}

static olist_item_t* olist_allocate(olist_t* olist) {
    const size_t item_size = offsetof(olist_item_t, payload) + olist->payload_size;

    for (unsigned int i = 0; i < olist->pool_size; i++) {
        olist_item_t* const item = (olist_item_t*)(((uint8_t*)olist->pool) + i * item_size);
        if (!item->active) {
            item->active = true;
            return item;
        }
    }
    return 0;
}

static void olist_insert_to_list(olist_t* olist, olist_item_t* item) {
    olist_item_t** itr = &olist->first;

    while (*itr && (*itr)->order <= item->order) {
        itr = &(*itr)->next;
    }

    item->next = *itr;
    *itr = item;
}

olist_item_t* olist_push(olist_t* olist, uint_fast32_t order, const void* payload) {
    olist_item_t* const allocated = olist_allocate(olist);
    if (allocated == 0) return 0;

    allocated->order = order;
    memcpy(allocated->payload, payload, olist->payload_size);

    olist_insert_to_list(olist, allocated);

    return allocated;
}

void* olist_pop(olist_t* olist, void* payload_buf, uint_fast32_t* order_buf) {
    if (olist->first == 0) return 0;

    olist_item_t* const item = olist->first;
    olist->first = item->next;

    if (payload_buf) memcpy(payload_buf, item->payload, olist->payload_size);
    if (order_buf) memcpy(order_buf, &item->order, sizeof(uint_fast32_t));

    item->active = false;

    return payload_buf;
}

// drop item from linked list but not set inactive state.
static void olist_drop_from_list(olist_t* olist, const olist_item_t* item) {
    if (olist->first == 0) return;

    if (olist->first == item) {
        olist->first = item->next;
        return;
    }

    for (olist_item_t* itr = olist->first; itr->next; itr = itr->next) {
        if (itr->next == item) {
            itr->next = item->next;
            return;
        }
    }
}

void olist_drop(olist_t* olist, olist_item_t* item) {
    if (!item->active) return;

    olist_drop_from_list(olist, item);
    item->active = false;
}

void olist_reorder(olist_t* olist, olist_item_t* item, uint_fast32_t new_order) {
    if (item->order == new_order || !item->active) return;

    olist_drop_from_list(olist, item);
    item->order = new_order;
    olist_insert_to_list(olist, item);
}

uint_fast32_t olist_item_count(const olist_t* olist) {
    uint_fast32_t count = 0;
    for (const olist_item_t* itr = olist->first; itr; itr = itr->next, count++);
    return count;
}
