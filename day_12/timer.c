#include <stdbool.h>

#include "timer.h"
#include "memory.h"


typedef struct TimeoutSetting {
    bool                   active;
    timeout_handler_t      handler;
    void*                  arg;
    uint32_t               timestamp;
    struct TimeoutSetting* next;
} timeout_setting_t;

#define TIMEOUT_MAX  512

typedef struct TimeoutManager {
    timeout_setting_t* first;
    timeout_setting_t  pool[TIMEOUT_MAX];
} timeout_manager_t;

static timeout_manager_t* _timeout_manager;
static uint32_t _current_time = 0;


void init_timeout_manager() {
    _timeout_manager = malloc(sizeof(timeout_manager_t));
    memset(_timeout_manager, 0x00, sizeof(timeout_manager_t));
}

void on_timer(const event_t* ev) {
    _current_time = ev->timer;

    while (_timeout_manager->first && _current_time >= _timeout_manager->first->timestamp) {
        _timeout_manager->first->handler(_timeout_manager->first->arg);

        _timeout_manager->first->active = false;
        _timeout_manager->first = _timeout_manager->first->next;
    }
}

static timeout_setting_t* allocate_timeout(timeout_setting_t setting) {
    for (int i = 0; i < TIMEOUT_MAX; i++) {
        if (_timeout_manager->pool[i].active == false) {
            _timeout_manager->pool[i] = setting;

            return &_timeout_manager->pool[i];
        }
    }
    return 0;
}

int set_timeout(timeout_handler_t handler, void* arg, uint32_t timeout) {
    const timeout_setting_t setting = {
        .active    = true,
        .handler   = handler,
        .arg       = arg,
        .timestamp = _current_time + timeout,
    };

    timeout_setting_t* const allocated = allocate_timeout(setting);
    if (allocated == 0) return -1;

    timeout_setting_t** itr = &_timeout_manager->first;

    while (*itr && (*itr)->timestamp < allocated->timestamp) {
        itr = &(*itr)->next;
    }

    allocated->next = *itr;
    *itr = allocated;

    return 1;
}
