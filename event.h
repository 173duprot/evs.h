#ifndef EVENT_SYSTEM_H
#define EVENT_SYSTEM_H

#include "../ecs.h/ecs.h"
#include "../mcmpq.h/mcmpq.h"
#include <pthread.h>
#include <stdlib.h>

#define MAX_EVENTS 1024
#define MAX_HANDLERS_PER_EVENT 16
#define EVENT_QUEUE_SIZE 64
#define MAX_THREADS 4  // Adjust the number of worker threads as needed

typedef uint8_t event_id_t;
typedef void (*event_callback_t)(struct ECS*, ent_t, void*);

typedef struct {
    event_id_t type;
    ent_t entity;
    void *data;
} event_t;

typedef struct {
    struct ECS *ecs;
    event_callback_t handlers[MAX_EVENTS][MAX_HANDLERS_PER_EVENT];
    size_t handler_count[MAX_EVENTS];
    queue_t event_queue;
    pthread_t workers[MAX_THREADS];
    _Atomic bool running;
} event_system_t;

// Create an event handler
static inline int event_system_create(event_system_t *event_system, event_id_t event_type, event_callback_t handler) {
    if (event_system->handler_count[event_type] >= MAX_HANDLERS_PER_EVENT) {
        return -1; // Too many handlers for this event
    }
    event_system->handlers[event_type][event_system->handler_count[event_type]++] = handler;
    return 0;
}

// Trigger an event
static inline void event_trigger(event_system_t *event_system, event_id_t event_type, ent_t entity, void *data) {
    event_t event = { .type = event_type, .entity = entity, .data = data };
    enqueue(&event_system->event_queue, &event);
}

// Worker thread function
static void* event_worker(void *arg) {
    event_system_t *event_system = (event_system_t*)arg;
    event_t event;

    while (atomic_load(&event_system->running)) {
        if (try_dequeue(&event_system->event_queue, &event)) {
            for (size_t i = 0; i < event_system->handler_count[event.type]; ++i) {
                event_system->handlers[event.type][i](event_system->ecs, event.entity, event.data);
            }
        }
    }

    return NULL;
}

// Initialize the event system
static inline void init_event_system(event_system_t *event_system, struct ECS *ecs) {
    event_system->ecs = ecs;
    memset(event_system->handlers, 0, sizeof(event_system->handlers));
    memset(event_system->handler_count, 0, sizeof(event_system->handler_count));
    atomic_store(&event_system->running, true);

    for (int i = 0; i < MAX_THREADS; ++i) {
        pthread_create(&event_system->workers[i], NULL, event_worker, event_system);
    }
}

// Destroy the event system
static inline void event_system_destroy(event_system_t *event_system) {
    atomic_store(&event_system->running, false);
    for (int i = 0; i < MAX_THREADS; ++i) {
        pthread_join(event_system->workers[i], NULL);
    }
}

#endif // EVENT_SYSTEM_H

