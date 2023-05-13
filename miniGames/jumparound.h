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
        void jump() { vel.y = 50.0f; };
    };

    // Scrolling stage for the jump around mini-game.
    class Stage {
        private:
            Physics::AABB* blocks;
            uint numBlocks;

            Physics::AABB goal; // The end of the platformer.
            Player player;

            float grav = -9.8f; // todo test for ideal gravity value

            // Restart the level. Called whenever the player dies.
            inline void restart();

        public:
            bool completed = 0; // Is the stage completed or not? 1 = yes, 0 = no. Cached for efficiency.

            Stage();

            void update(float dt);

            void draw() const;
    };
}

#endif // !JUMP_AROUND_H
