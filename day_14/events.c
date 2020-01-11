#include "events.h"
#include "memory.h"


#define EVENT_QUEUE_SIZE  512

typedef struct EventQueue {
    event_t       queue[EVENT_QUEUE_SIZE];
    uint_fast16_t read_index, write_index;
} event_queue_t;


static event_queue_t* _event_queue;

void init_event_queue() {
    _event_queue = malloc(sizeof(event_queue_t));
    memset(_event_queue, 0x00, sizeof(event_queue_t));
}

const event_t* peek_event() {
    const event_t* const ev = &_event_queue->queue[_event_queue->read_index];

    return ev->active ? ev : 0;
}

const event_t* pop_event() {
    event_t* const ev = &_event_queue->queue[_event_queue->read_index];

    if (!ev->active) {
        return 0;
    }

    ev->active = false;
    _event_queue->read_index = (_event_queue->read_index + 1) % EVENT_QUEUE_SIZE;
    return ev;
}

void push_event(event_t ev) {
    if (_event_queue->queue[_event_queue->write_index].active) return;  // drop event if failed to push

    _event_queue->queue[_event_queue->write_index] = ev;
    _event_queue->queue[_event_queue->write_index].active = true;
    _event_queue->write_index = (_event_queue->write_index + 1) % EVENT_QUEUE_SIZE;
}
