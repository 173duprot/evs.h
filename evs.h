#ifndef EVS_H
#define EVS_H

#include "../mcmpq.h/mcmpq.h" // git clone https://github.com/173duprot/mcmpq.h
#include <pthread.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>

#define MAX_EVT 1024 // Max Events
#define MAX_SUB 16   // Max Subscriptions per Event
#define MAX_THR 4    // Max Thread Count
#define EVT_QUEUE 64 // Queue Size

typedef uint8_t evt_id_t;
typedef void (*evt_sub_t)(void*, void*);

typedef struct {
    evt_id_t type;
    void *context;
    void *data;
} evt_t;

typedef struct {
    evt_sub_t subscribers[MAX_EVT][MAX_SUB];
    size_t sub_count[MAX_EVT];
    queue_t evt_queue;
    pthread_t workers[MAX_THR];
    _Atomic bool running;
} evs_t;

// Add a new event subscriber
static inline int evs_subscribe(evs_t *evs, evt_id_t type, evt_sub_t subscriber) {
    size_t count = evs->sub_count[type];
    if (count >= MAX_SUB) return -1;
    evs->subscribers[type][count] = subscriber;
    evs->sub_count[type]++;
    return 0;
}

// Trigger an event
static inline void ev_trigger(evs_t *evs, evt_id_t type, void *context, void *data) {
    evt_t event = { .type = type, .context = context, .data = data };
    enqueue(&evs->evt_queue, &event);
}

// Worker thread function
static void* evs_worker(void *arg) {
    evs_t *sys = (evs_t*)arg;
    evt_t event;
    while (atomic_load(&sys->running)) {
        if (try_dequeue(&sys->evt_queue, &event)) {
            for (size_t i = 0; i < sys->sub_count[event.type]; ++i) {
                sys->subscribers[event.type][i](event.context, event.data);
            }
        }
    }
    return NULL;
}

// Initialize the event system
static inline void evs_init(evs_t *evs) {
    memset(evs->subscribers, 0, sizeof(evs->subscribers));
    memset(evs->sub_count, 0, sizeof(evs->sub_count));
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
