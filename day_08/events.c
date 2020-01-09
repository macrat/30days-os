#include "events.h"

static event_queue_t event_queue = {0};

event_t* peek_event() {
    event_t *ev = &event_queue.queue[event_queue.read_index];

    return ev->active ? ev : 0;
}

event_t* pop_event() {
    event_t *ev = &event_queue.queue[event_queue.read_index];

    if (!ev->active) {
        return 0;
    }

    ev->active = 0;
    event_queue.read_index = (event_queue.read_index + 1) % EVENT_QUEUE_SIZE;
    return ev;
}

void push_event(event_t ev) {
    event_queue.queue[event_queue.write_index] = ev;
    event_queue.queue[event_queue.write_index].active = 1;
    event_queue.write_index = (event_queue.write_index + 1) % EVENT_QUEUE_SIZE;
}
