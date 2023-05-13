#ifndef PING_H
#define PING_H

#include "../utility.h"
#include "../physics.h"

namespace Ping {
    // todo set up the dimensions for the Paddle
    // todo setup an AABB hitbox for the paddles
    // todo setup a radius for the Ball

    struct Paddle {
        ZMath::Vec2D pos; // Position of the paddle.
        float yVel = 0.0f; // Y velocity of the paddle.
    };

    struct Ball {
        ZMath::Vec2D pos; // Position of the ball.
        ZMath::Vec2D vel; // Velocity of the ball.
        ZMath::Vec2D dir; // Normalized velocity of the ball. Used for detecting collisions for the ball.
    };

    // Randomly selected stage for the Ping mini-game.
    class Stage {
        // * Tile Coordinate System
        // (0, 0), (1, 0), (2, 0), ..., (n, 0)
        // (0, 1), (1, 1), (2, 1), ..., (n, 1)
        // (0, 2), (1, 2), (2, 2), ..., (n, 2)
        //   ...     ...     ...   ...,  ...
        // (0, n), (1, n), (2, n), ..., (n, n)

        public:
            // width = numCols
            // height = numRows
            static const uint WIDTH = 50; // Width of the board in tile coordinates.
            static const uint HEIGHT = 38; // Height of the board in tile coordinates.
            uint numPins; // todo figure out a good value to make this

        private:
            Sprite grid[HEIGHT][WIDTH];
            Ball ball;
            Paddle player; // Player's paddle.

            Physics::AABB* walls; // Walls on the stage.
            uint numWalls;

            uint numPaddles;
            Paddle* paddles; // enemy paddles

            ZMath::Vec2D* pins;

        public:
            Stage();

            // Update the positions and any pins that should be knocked down.
            void update(float dt) {
                // * Information for updating the balls.
                ZMath::Vec2D dP = ball.vel * dt;
                float dPSq = dP.magSq();
                float dist;
                bool yAxis;

                Physics::Ray2D ray(ball.pos, ball.dir);

                // ! This will not properly detect collisions due to the break statement.
                // todo fix later
                for (uint i = 0; i < numWalls; ++i) {
                    if (Physics::raycast(ray, walls[i], dist, yAxis)) {
                        if (dPSq >= dist*dist) {
                            if (yAxis) { ball.vel.x = -ball.vel.x; ball.dir.x = -ball.dir.x; }
                            else { ball.vel.y = -ball.vel.y; ball.dir.y = -ball.dir.y; }
                        }

                        break;
                    }
                }

                // todo check for paddle collisions

                ball.pos += dP;
            };

            // Draw the tiles associated with the stage.
            void draw() const;
    };
}

#endif // !PING_H
