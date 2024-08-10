#include "event.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define NUM_ENTITIES 1000
#define NUM_EVENTS_PER_ENTITY 1000
#define TEST_DURATION_SECONDS 10
#define EVENT_TYPES 3

// Event handlers
void on_frame_update(struct ECS *ecs, ent_t entity, void *data) {
    // Simulate work done during frame update
    // Frame update logic here
   // puts("frame update");
}

void on_position_update(struct ECS *ecs, ent_t entity, void *data) {
    // Simulate work done during position update
    // Position update logic here
    //puts("position update");
}

void on_collision_detected(struct ECS *ecs, ent_t entity, void *data) {
    // Simulate work done during collision detection
    // Collision detection logic here
    //puts("collision detection");
}

int main() {
    struct ECS ecs = {0};  // Initialize the ECS system as needed
    event_system_t event_system;

    init_event_system(&event_system, &ecs);

    // Create entities and register event handlers
    for (int i = 0; i < NUM_ENTITIES; ++i) {
        ent_t entity = create(&ecs);

        // Register different handlers for each event type
        event_system_create(&event_system, 1, on_frame_update);     // FrameUpdateEvent
        event_system_create(&event_system, 2, on_position_update);   // PositionUpdateEvent
        event_system_create(&event_system, 3, on_collision_detected); // CollisionDetectedEvent
    }

    // Start time for the test
    time_t start_time = time(NULL);

    size_t event_count = 0;

    // Main game loop
    while (difftime(time(NULL), start_time) < TEST_DURATION_SECONDS) {
        for (int i = 0; i < NUM_EVENTS_PER_ENTITY; ++i) {
            for (int j = 0; j < EVENT_TYPES; ++j) {
                for (int k = 0; k < NUM_ENTITIES; ++k) {
                    // Emit different events
                    event_trigger(&event_system, j + 1, k, NULL);
                    event_count++;
                }
            }
        }
    }

    // End time for the test
    time_t end_time = time(NULL);

    // Performance metrics
    double duration = difftime(end_time, start_time);
    printf("Processed %zu events in %.2f seconds\n", event_count, duration);
    printf("Events per second: %.2f\n", event_count / duration);

    event_system_destroy(&event_system);

    return 0;
}

