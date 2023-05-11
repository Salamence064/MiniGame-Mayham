#ifndef TRICKSHOT_H
#define TRICKSHOT_H

#include <string>
typedef unsigned int uint; // probs move this into the same primary file as sprite and the string include

// * =======================
// * Trick Shot Backend
// * =======================

// todo move to a primary file and include it for this file
struct Sprite {
    std::string filepath;
    uint x = 0;
    uint y = 0;
    static const uint width = 16;
    static const uint height = 16;
};

namespace TrickShot {
    struct Ball {
        uint x, y; // coordinates in terms of pixels
        uint xVel, yVel; // x and y velocity in terms of pixels
        static const uint linearDamping = 0.9; // friction applied to the ball
        // todo test for an ideal linearDamping value
    };

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
            static const uint WIDTH = 20; // width of the board
            static const uint HEIGHT = 20; // height of the board

            /**
             * @brief Shoot the ball in the direction determined by the player releasing the mouse.
             * 
             * @param mouseX X position of the mouse in pixels.
             * @param mouseY Y position of the mouse in pixels.
             * @return (bool) 1 if the shot made it in, 0 otherwise.
             */
            bool shoot(uint mouseX, uint mouseY);

            /**
             * @brief Update the position of the ball while its velocity is not 0.
             * 
             * @return 1 while the velocity > 0 and 0 once the velocity reaches 0.
             */
            bool update() const;

            /**
             * @brief Draw the power-bar and arrow while the player is readying his/her shot.
             * 
             * @param mouseX X position of the mouse in pixels.
             * @param mouseY Y position of the mouse in pixels.
             */
            void drawShootingUI(uint mouseX, uint mouseY) const;

            /**
             * @brief Draw the tiles associated with the stage.
             * 
             */
            void draw() const;

        private:
            Sprite grid[HEIGHT][WIDTH];
            Ball ball; // ball's coordinates in terms of pixels
            uint cupX, cupY; // cup's coordinates in terms of tile number
    };
}

#endif // !TRICKSHOT_H
