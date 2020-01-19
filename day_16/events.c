#include "events.h"
#include "queue.h"


#define EVENT_QUEUE_SIZE  512


static queue_t* _event_queue;
static task_t observer = {0};


void init_event_queue() {
    _event_queue = create_queue(EVENT_QUEUE_SIZE, sizeof(event_t));
}

const event_t* pop_event(event_t* buf) {
    return queue_pop(_event_queue, buf);
}

void push_event(event_t ev) {
    queue_push(_event_queue, &ev);

    if (!_event_queue->first->next && observer.selector) task_wakeup(observer);
}

void observe_event(task_t task) {
    observer = task;
}
