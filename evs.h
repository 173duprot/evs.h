#ifndef EVS_H
#define EVS_H

#include "../mcmpq.h/mcmpq.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#define MAX_EVT 1024
#define MAX_HDL 16
#define EVT_QUEUE 64
#define MAX_THR 4

typedef uint8_t evt_id_t;
typedef void (*evt_cb_t)(void*, void*);

typedef struct {
    evt_id_t type;
    void *ctx;
    void *data;
} evt_t;

typedef struct {
    evt_cb_t handlers[MAX_EVT][MAX_HDL];
    size_t hdl_count[MAX_EVT];
    queue_t evt_queue; // Presumed to be thread-safe
    pthread_t workers[MAX_THR];
    _Atomic bool running;
} evs_t;

// Add a new event handler
static inline int evs_add_hdl(evs_t *evs, evt_id_t type, evt_cb_t handler) {
    size_t count = evs->hdl_count[type];
    if (count >= MAX_HDL) return -1;
    evs->handlers[type][count] = handler;
    evs->hdl_count[type]++;
    return 0;
}

// Trigger an event
static inline void evs_trigger(evs_t *evs, evt_id_t type, void *ctx, void *data) {
    evt_t event = { .type = type, .ctx = ctx, .data = data };
    enqueue(&evs->evt_queue, &event);
}

// Worker thread function
static void* evs_worker(void *arg) {
    evs_t *sys = (evs_t*)arg;
    evt_t event;
    while (atomic_load(&sys->running)) {
        if (try_dequeue(&sys->evt_queue, &event)) {
            for (size_t i = 0; i < sys->hdl_count[event.type]; ++i) {
                sys->handlers[event.type][i](event.ctx, event.data);
            }
        }
    }
    return NULL;
}

// Initialize the event system
static inline void evs_init(evs_t *evs) {
    memset(evs->handlers, 0, sizeof(evs->handlers));
    memset(evs->hdl_count, 0, sizeof(evs->hdl_count));
    atomic_store(&evs->running, true);
    for (int i = 0; i < MAX_THR; ++i) {
        pthread_create(&evs->workers[i], NULL, evs_worker, evs);
    }
}

// Destroy the event system
static inline void evs_destroy(evs_t *evs) {
    atomic_store(&evs->running, false);
    for (int i = 0; i < MAX_THR; ++i) {
        pthread_join(evs->workers[i], NULL);
    }
}

#endif // EVS_H
