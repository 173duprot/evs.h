#include "evs.h"
#include "../ecs.h/ecs.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define NUM_ENT 512
#define NUM_EVT_PER_ENT 1000
#define TEST_SEC 10
#define EVT_TYPES 3

// Event handlers
void on_frame(void *ctx, void *data) {
    // Frame update logic here
}

void on_position(void *ctx, void *data) {
    // Position update logic here
}

void on_collision(void *ctx, void *data) {
    // Collision detection logic here
}

int main() {
    evs_t evs;
    ecs_t ecs = {0};

    evs_init(&evs);

    // Subscribe <event_id> to <func>
    evs_subscribe(&evs, 1, on_frame);
    evs_subscribe(&evs, 1, on_position);
    evs_subscribe(&evs, 2, on_position);
    evs_subscribe(&evs, 3, on_collision);

    // Create entities
    for (int i = 0; i < NUM_ENT; ++i) {
        create_ent(&ecs);
    }

    // Start time for the test
    time_t start = time(NULL);
    size_t evt_count = 0;

    // Main loop
    while (difftime(time(NULL), start) < TEST_SEC) {
        for (int i = 0; i < NUM_EVT_PER_ENT; ++i) {
            for (int j = 0; j < EVT_TYPES; ++j) {
                for (int k = 0; k < NUM_ENT; ++k) {
                    // Emit different events
                    ev_trigger(&evs, j + 1, &ecs, &ecs.data[0][k]);
                    evt_count++;
                }
            }
        }
    }

    // End time for the test
    time_t end = time(NULL);

    // Performance metrics
    double duration = difftime(end, start);
    printf("Processed %zu events in %.2f seconds\n", evt_count, duration);
    printf("Events per second: %.2f\n", evt_count / duration);

    evs_destroy(&evs);

    return 0;
}
