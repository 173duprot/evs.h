#include "event.h"
#include <stdio.h>

// Example event handlers
void on_frame_update(struct ECS *ecs, ent_t entity, void *data) {
    // Process frame update logic here
    puts("frame");
}

void on_position_update(struct ECS *ecs, ent_t entity, void *data) {
    // Process position update logic here
    puts("position");
}

int main() {
    struct ECS ecs = {0};  // Initialize the ECS system as needed
    event_system_t event_system;

    init_event_system(&event_system, &ecs);

    // Register multiple handlers for the same event
    event_system_create(&event_system, 1, on_frame_update);  // FrameUpdateEvent
    event_system_create(&event_system, 1, on_position_update);  // PositionUpdateEvent

    // Emit a FrameUpdateEvent for entity 0
    event_trigger(&event_system, 1, 0, NULL);

    // Main game loop
    while (1) {
        event_trigger(&event_system, 1, 0, NULL);  // Emit FrameUpdateEvent
    }

    event_system_destroy(&event_system);

    return 0;
}

