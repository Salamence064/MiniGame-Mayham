#ifndef JUMP_AROUND_H
#define JUMP_AROUND_H

#include "../utility.h"
#include "../physics.h"

// todo update to model the player's hitboxes with ellipses
// todo possibly add in enemies

namespace JumpAround {
    struct Player {
        ZMath::Vec2D pos; // position of the player
        ZMath::Vec2D vel; // player velocity
        Physics::Circle hitbox; // update to be an ellipse

        // Make the player jump. Called everytime the player touches the ground.
        // todo test for an ideal initial y velocity.
        void jump() { vel.y = 20.0f; };
    };

    // Scrolling stage for the jump around mini-game.
    class Stage {
        private:
            ZMath::Vec2D origin; // initial position of the player

            Physics::AABB* blocks;
            uint numBlocks;

            Physics::AABB* ground;
            uint numGround;

            Physics::AABB goal; // The end of the platformer.
            Player player;

            float grav = -9.8f; // todo test for ideal gravity value
            float xOffset = 0.0f; // Horizontal offset for the player with respect to the full map.

            // Restart the level. Called whenever the player dies.
            inline void restart() {
                player.pos = origin;
                player.hitbox.c = origin;
                player.vel.zero();
                xOffset = 0.0f;
            };

        public:
            bool completed = 0; // Is the stage completed or not? 1 = yes, 0 = no. Cached for efficiency.

            Stage();

            // Move the player. Left parameter determines if the player is moving left or right.
            void move(bool left) {
                // todo determine an appropriate speed value
                if (left) { player.vel.x = -20.0f; }
                else { player.vel.y = 20.0f; }
            };

            void update(float dt) {
                // check if the player needs to jump
                for (uint i = 0; i < numGround; ++i) {
                    if (Physics::CircleAndAABB(player.hitbox, ground[i])) {
                        player.jump();
                        break;
                    }
                }

                // todo add collision checks with walls
                // todo add checks for falling to your death (check if the player touches 0 y level)

                // update the player
                player.vel.y += grav * dt;
                player.pos += player.vel * dt;
            };

            void draw() const;
    };
}

#endif // !JUMP_AROUND_H
