//
// Created by Jayesh Gajbhar on 10/23/24.
//

#ifndef EVENT_HPP
#define EVENT_HPP

#include "../ECS/types.hpp"
#include "../model/components.hpp"

// all the events in the game engine will inherit from this class
enum EventType {
    EntityRespawn,
    EntityDeath,
    EntityCollided,
    EntityInput,
    EntityTriggered
};

struct EntityRespawnData {
    Entity entity;
};

struct EntityDeathData {
    Entity entity;
    Transform respawnPosition;
};

struct EntityCollidedData {
    Entity entityA;
    Entity entityB;
};

struct EntityTriggeredData {
    Entity triggerEntity;
    Entity otherEntity;
};

struct EntityInputData {
    Entity entity;
    SDL_Keycode key;
};

struct Event {
    EventType type;
    std::variant<EntityRespawnData, EntityDeathData, EntityCollidedData, EntityInputData, EntityTriggeredData> data;
};


#endif //EVENT_HPP
