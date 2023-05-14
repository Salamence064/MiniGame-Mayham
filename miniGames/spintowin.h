#ifndef SPIN_TO_WIN_H
#define SPIN_TO_WIN_H

#include "../utility.h"
#include "../physics.h"

namespace SpinToWin {
    // todo either make 3D prespectived or 2D top-down
    // todo update players to use ellipses

    enum Dir {
        UP,
        RIGHT,
        DOWN,
        LEFT
    };

    class Player {
        public:
            static const float speed = 7.5f;

            ZMath::Vec2D vel = ZMath::Vec2D();
            ZMath::Vec2D pos;
            Physics::Circle hitbox;

            Player(const ZMath::Vec2D &pos) : pos(pos), hitbox(Physics::Circle(pos, 8.0f)) {};

            void update(float dt) { pos += vel * dt; };

            void draw() const;
    };

    class Stage {
        // * Section labeling
        // --- | --- | ---
        // -1- | -2- | -3-
        // --- | --- | ---
        // --- | --- | ---
        // -4- | -5- | -6-
        // --- | --- | ---
        // --- | --- | ---
        // -7- | -8- | -9-
        // --- | --- | ---

        public:
            static const uint WIDTH = 9;
            static const uint HEIGHT = 9;
            static const uint BLOCK_WIDTH = 3;
            static const uint BLOCK_HEIGHT = 3;

        private:
            Sprite grid[HEIGHT][WIDTH];
            Player player1, player2;

            float bounceBack = 24.0f; // todo test for a good value

        public:
            Stage();

            // player: 0 = P1, 1 = P2
            void move(bool player, Dir dir) {
                if (player) { // P2
                    switch(dir) {
                        case Dir::UP: {
                            player2.vel.y = Player::speed;
                            break;
                        }

                        case Dir::RIGHT: {
                            player2.vel.x = Player::speed;
                            break;
                        }

                        case Dir::DOWN: {
                            player2.vel.y = -Player::speed;
                            break;
                        }

                        case Dir::LEFT: {
                            player2.vel.x = -Player::speed;
                            break;
                        }
                    }

                    return;
                }

                // P1
                switch(dir) {
                    case Dir::UP: {
                        player1.vel.y = Player::speed;
                        break;
                    }

                    case Dir::RIGHT: {
                        player1.vel.x = Player::speed;
                        break;
                    }

                    case Dir::DOWN: {
                        player1.vel.y = -Player::speed;
                        break;
                    }

                    case Dir::LEFT: {
                        player1.vel.x = -Player::speed;
                        break;
                    }
                }
            };

            // player: 0 = P1, 1 = P2
            void stop(bool player, Dir dir) {
                if (player) { // P2
                    switch(dir) {
                        case Dir::UP:
                        case Dir::DOWN:
                            player2.vel.y = 0;
                            break;

                        case Dir::RIGHT:
                        case Dir::LEFT:
                            player2.vel.x = 0;
                            break;
                    }

                    return;
                }

                // P1
                switch(dir) {
                    case Dir::UP:
                    case Dir::DOWN:
                        player1.vel.y = 0;
                        break;

                    case Dir::RIGHT:
                    case Dir::LEFT:
                        player1.vel.x = 0;
                        break;
                }
            };

            void update(float dt) {
                // todo add checks to make sure the player is not falling off the stage

                player1.update(dt);
                player2.update(dt);

                // check for collisions between the players
                // todo later add a check for who is more in the direction of the collision and apply more bounceback to the person less so 
                // (get the collision normal and dot it with the player's vel, the greater value is the one that has more in the same direction)
                if (Physics::CircleAndCircle(player1.hitbox, player2.hitbox)) {
                    // todo update it in the direction of the collision normal, too
                    // this is just temp code as this would not work
                    player1.pos -= bounceBack;
                    player2.pos -= bounceBack;
                }
            };

            void draw() const;
    };
}

#endif // !SPIN_TO_WIN_H
