//
// Created by Utsav Lal on 10/6/24.
//

#include "../ECS/coordinator.hpp"
#include "../ECS/system.hpp"
#include "../model/components.hpp"
#include <thread>

extern Coordinator gCoordinator;

class KeyboardMovementSystem : public System {
public:
    void update() {
        // std::lock_guard<std::mutex> lock(update_mutex);
        for (const auto entity: entities) {
            auto &transform = gCoordinator.getComponent<Transform>(entity);
            auto &kinematic = gCoordinator.getComponent<CKinematic>(entity);
            auto &keyboard = gCoordinator.getComponent<KeyboardMovement>(entity);
            auto &jump = gCoordinator.getComponent<Jump>(entity);

            const Uint8 *state = SDL_GetKeyboardState(nullptr);
            kinematic.velocity.x = 0;
            // kinematic.velocity.y = 0;
            // if (state[SDL_SCANCODE_W]) {
            //     kinematic.velocity.y = -keyboard.speed;
            // }
            if (state[SDL_SCANCODE_A]) {
                kinematic.velocity.x = -keyboard.speed;
            }
            // if (state[SDL_SCANCODE_S]) {
            //     kinematic.velocity.y = keyboard.speed;
            // }
            if (state[SDL_SCANCODE_D]) {
                kinematic.velocity.x = keyboard.speed;
            }
            if (state[SDL_SCANCODE_SPACE] && !jump.isJumping && jump.canJump) {
                // Initialize jump
                jump.isJumping = true;
                jump.jumpTime = 0.f;
                jump.canJump = false;
                kinematic.velocity.y = -jump.initialJumpVelocity; // Add an initial upward velocity
            }
        }
    }
};
