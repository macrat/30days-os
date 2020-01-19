#include <stdlib.h>
#include <string.h>

#include "queue.h"


size_t queue_sizeof(size_t pool_size, size_t payload_size) {
    return offsetof(queue_t, pool) + (offsetof(queue_item_t, payload) + payload_size) * pool_size;
}

queue_t* create_queue_on(void* dest, size_t pool_size, size_t payload_size) {
    memset(dest, 0x00, queue_sizeof(pool_size, payload_size));

    queue_t* queue = (queue_t*)dest;

    queue->pool_size = pool_size;
    queue->payload_size = payload_size;

    return queue;
}

queue_t* create_queue(size_t pool_size, size_t payload_size) {
    void* buf = malloc(queue_sizeof(pool_size, payload_size));
    if (buf == 0) return 0;

    return create_queue_on(buf, pool_size, payload_size);
}

static queue_item_t* queue_allocate(queue_t* queue) {
    const size_t item_size = offsetof(queue_item_t, payload) + queue->payload_size;

    for (unsigned int i = 0; i < queue->pool_size; i++) {
        queue_item_t* const item = (queue_item_t*)(((uint8_t*)queue->pool) + i * item_size);
        if (!item->active) {
            item->active = true;
            return item;
        }
    }
    return 0;
}

queue_item_t* queue_push(queue_t* queue, const void* data) {
    queue_item_t* const allocated = queue_allocate(queue);
    if (!allocated) return 0;

    memcpy(allocated->payload, data, queue->payload_size);

    if (queue->end) queue->end->next = allocated;

    queue->end = allocated;
    allocated->next = 0;

    if (!queue->first) queue->first = allocated;

    return allocated;
}

void* queue_pop(queue_t* queue, void* buf) {
    if (!queue->first) return 0;

    queue_item_t* const item = queue->first;

    if (buf) memcpy(buf, item->payload, queue->payload_size);

    queue->first = item->next;
    item->active = false;

    if (queue->end == item) queue->end = 0;

    return buf;
}

void* queue_pop_and_push(queue_t* queue, void* buf) {
    if (!queue->first) return 0;

    queue_item_t* const item = queue->first;

    memcpy(buf, item->payload, queue->payload_size);

    if (!item->next) return buf;

    queue->first = item->next;
    queue->end->next = item;
    queue->end = item;
    item->next = 0;

    return buf;
}

static bool is_same_data(const void* a, const void* b, size_t size) {
    for (unsigned int i = 0; i < size; i++) {
        if (((uint8_t*)a)[i] != ((uint8_t*)b)[i]) return false;
    }
    return true;
}

int queue_drop_by_data(queue_t* queue, const void* data) {
    if (!queue->first) return -1;

    if (is_same_data(queue->first->payload, data, queue->payload_size)) {
        queue_item_t* const item = queue->first;

        if (item == queue->end) {
            queue->first = queue->end = 0;
        } else {
            queue->first = item->next;
        }
        item->next = 0;
        item->active = false;

        return 0;
    }

    for (queue_item_t* itr = queue->first; itr->next; itr = itr->next) {
        if (is_same_data(itr->next->payload, data, queue->payload_size)) {
            queue_item_t* const item = itr->next;

            if (item == queue->end) {
                queue->end = itr;
                itr->next = 0;
            } else {
                itr->next = item->next;
            }
            item->next = 0;
            item->active = false;

            return 0;
        }
    }

    return 1;
}

int queue_item_count(const queue_t* queue) {
    int count = 0;

    for (queue_item_t* itr = queue->first; itr; itr = itr->next) {
        count++;
    }

    return count;
}
