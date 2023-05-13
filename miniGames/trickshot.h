#ifndef TRICKSHOT_H
#define TRICKSHOT_H

#include <fstream>
#include "../utility.h"
#include "../physics.h"

// * =======================
// * Trick Shot Backend
// * =======================

// todo handle graphics after adding the graphics library

namespace TrickShot {
    struct Ball {
        ZMath::Vec2D pos; // ball's position in terms of pixels
        ZMath::Vec2D vel; // ball's velocity in terms of pixels
        ZMath::Vec2D dir; // normalized direction of the ball -- cached for efficiency
        static const float linearDamping = 0.9; // friction applied to the ball
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

            bool complete = 0; // has the stage been completed

        private:
            Sprite grid[HEIGHT][WIDTH];
            Ball ball; // The ball the player shoots.
            Physics::AABB cup; // AABB representing the cup. This should lay in one tile.

            Physics::AABB* walls; // List of walls the player can collide with.
            uint numWalls = 0; // number of walls

        public:
            // todo add one more block type

            /** 
             * Symbol Legend:
             * 
             * space = nothing
             * w = wall
             * b = ball
             * c = cup
             * 
             * Each can be followed by an RGB to shade it a different color in the format [r],[b],[g]
             */

            // Instantiate a stage object.
            // This will randomly select one of the possible stages for the minigame.
            Stage() {
                std::ifstream f("trickshot1.map");
                std::string line;

                for (uint i = 0; i < HEIGHT; ++i) {
                    getline(f, line);

                    for (uint j = 0; j < WIDTH; ++j) {
                        if (line[j] == ' ') { continue; } // blank space

                        std::string filepath = "";

                        switch (line[j]) {
                            case 'w': {
                                filepath = "wall.png";
                                break;
                            }

                            case 'b': {
                                filepath = "ball.png";
                                ball = {ZMath::Vec2D(j*16.0f, i*16.0f), ZMath::Vec2D(), ZMath::Vec2D()};
                                break;
                            }

                            case 'c': {
                                filepath = "cup.png";
                                float x = j*16.0f, y = i*16.0f;
                                cup = Physics::AABB(ZMath::Vec2D(x, y), ZMath::Vec2D(x + 16.0f, y + 16.0f));
                                break;
                            }
                        }

                        grid[i][j] = (Sprite) {filepath, j * 16.0f, i * 16.0f};

                        // todo add in the RGB part of the parser
                    }
                }
            };

            /**
             * @brief Shoot the ball in the direction determined by the player releasing the mouse.
             * 
             * @param mousePos Relative position of the mouse in terms of pixels.
             */
            void shoot(const ZMath::Vec2D &mousePos) {
                ZMath::Vec2D diff = (ball.pos - mousePos) * 0.5f;
                ZMath::Vec2D dir = diff.getSigns();

                ball.vel.set(diff.x * dir.x, diff.y * dir.y);
                ball.dir.set(ball.vel.normalize());

                // todo update with better values once testing can be done
            };

            /**
             * @brief Update the position of the ball while its velocity is not 0.
             * 
             * @param dt The time step passed. This should be standardized by the physics engine for determinism.
             * @return 1 while the magnitude of the velocity is greater than 0.35 and 0 once its magnitude reaches that cut-off.
             */
            bool update(float dt) {
                float dP = ball.vel * dt * ball.vel;
                float dist;
                bool yAxis;

                Physics::Ray2D ray(ball.pos, ball.dir);

                // ! This will currently not properly detect collisions due to the ordering of the AABBs mattering (due to break)
                for (uint i = 0; i < numWalls; ++i) {
                    if (Physics::raycast(ray, walls[i], dist, yAxis)) {
                        if (dP >= dist) {
                            if (yAxis) { ball.vel.x = -ball.vel.x; ball.dir.x = -ball.dir.x; }
                            else { ball.vel.y = -ball.vel.y; ball.dir.y = -ball.dir.y; }
                        }

                        break;
                    }
                }

                // todo test values for this (both the slowing and the lenience for letting it go in)
                if (Physics::raycast(ray, cup, dist, yAxis) && dP >= dist) {
                    if (ball.vel.magSq() <= 6.25) { complete = 1; return 0; }
                    ball.vel *= 0.45;
                }

                ball.pos += dP;
                ball.vel *= ball.linearDamping;

                return ball.vel.magSq() >= 0.125;
            };

            /**
             * @brief Draw the power-bar and arrow while the player is readying his/her shot.
             * 
             * @param mousePos The relative position of the mouse in terms of pixels.
             */
            void drawShootingUI(ZMath::Vec2D const &mousePos) const;

            // Draw the tiles associated with the stage.
            void draw() const;
    };
}

#endif // !TRICKSHOT_H
