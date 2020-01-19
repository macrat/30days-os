#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct QueueItem {
    bool              active;
    struct QueueItem* next;
    uint8_t           payload[];
} queue_item_t;


typedef struct Queue {
    size_t        pool_size, payload_size;
    queue_item_t* first;
    queue_item_t* end;
    queue_item_t  pool[];
} queue_t;


extern size_t queue_sizeof(size_t pool_size, size_t payload_size);
extern queue_t* create_queue_on(void* dest, size_t pool_size, size_t payload_size);
extern queue_t* create_queue(size_t pool_size, size_t payload_size);

extern queue_item_t* queue_push(queue_t* queue, const void* data);
extern void* queue_pop(queue_t* queue, void* buf);
extern void* queue_pop_and_push(queue_t* queue, void* buf);
extern int queue_drop_by_data(queue_t* queue, const void* data);

extern int queue_item_count(const queue_t* queue);
