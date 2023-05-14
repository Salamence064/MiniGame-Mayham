#ifndef SPIN_TO_WIN_H
#define SPIN_TO_WIN_H

#include "../utility.h"
#include "../physics.h"

namespace SpinToWin {
    // todo either make 3D prespectived or 2D top-down

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

            Player(const ZMath::Vec2D &pos) : pos(pos) {};

            void update(float dt);

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

            float bounceBack; // todo test for a good value

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

            void update(float dt);

            void draw() const;
    };
}

#endif // !SPIN_TO_WIN_H
