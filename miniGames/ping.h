#ifndef PING_H
#define PING_H

#include "../utility.h"
#include "../physics.h"

namespace Ping {
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

            uint numPaddles;
            Paddle* paddles;

            ZMath::Vec2D* pins;

        public:
            Stage();

            // Update the positions and any pins that should be knocked down.
            void update(float dt);

            // Draw the tiles associated with the stage.
            void draw() const;
    };
}

#endif // !PING_H
