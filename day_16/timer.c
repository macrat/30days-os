#include <stdbool.h>

#include "olist.h"
#include "timer.h"


typedef struct TimeoutClojure {
    timeout_handler_t handler;
    void*             arg;
} timeout_clojure_t;

#define TIMEOUT_MAX_NUM  512

static olist_t* _timeout_manager;
static uint32_t _current_time = 0;


void init_timeout_manager() {
    _timeout_manager = create_olist(TIMEOUT_MAX_NUM, sizeof(timeout_clojure_t));
}

static inline void call_timeout_clojure(timeout_clojure_t* clojure) {
    if (clojure) clojure->handler(clojure->arg);
}

void on_timer_count(uint32_t count) {
    _current_time = count;

    timeout_clojure_t clojure;

    while (_timeout_manager->first && _timeout_manager->first->order <= _current_time) {
        call_timeout_clojure(olist_pop(_timeout_manager, &clojure, 0));
    }
}

int set_timeout(timeout_handler_t handler, void* arg, uint32_t timeout) {
    return olist_push(_timeout_manager, _current_time + timeout, &(timeout_clojure_t){
        .handler = handler,
        .arg     = arg,
    }) ? 0 : -1;
}
