#ifndef TRICKSHOT_H
#define TRICKSHOT_H

#include <iostream> // !debugging
#include <fstream>
#include "../utility.h"
#include "../physics.h"

// * =======================
// * Trick Shot Backend
// * =======================

// todo handle graphics after adding the graphics library

namespace TrickShot {
    // todo setup a radius for the ball
    // todo factor in the ball's radius into the calculations for reversing its velocity (for both x and y vels)

    struct Ball {
        ZMath::Vec2D pos; // ball's position in terms of pixels
        ZMath::Vec2D vel; // ball's velocity in terms of pixels
        ZMath::Vec2D dir; // normalized direction of the ball -- cached for efficiency
        float linearDamping = 0.9; // friction applied to the ball
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
            // todo Add rule of three stuff but with GPU commands called instead of memory ones. 
                // todo Rule of 5 Not applicable as there isn't a move operator for the GPU stuff.

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
                std::ifstream f("miniGames/resources/trickshot/maps/map1.map");

                std::string line;

                for (uint i = 0; i < HEIGHT; ++i) {
                    getline(f, line);

                    Image image;

                    for (uint j = 0; j < WIDTH; ++j) {
                        if (line[j] == ' ') { grid[i][j].exists = 0; continue; } // blank space

                        switch (line[j]) {
                            case 'w': {
                                image = LoadImage("miniGames/resources/trickshot/ball.png");
                                break;
                            }

                            case 'b': {
                                image = LoadImage("miniGames/resources/trickshot/ball.png");
                                ball = {ZMath::Vec2D(j*16.0f, i*16.0f), ZMath::Vec2D(), ZMath::Vec2D()};
                                break;
                            }

                            case 'c': {
                                image = LoadImage("miniGames/resources/trickshot/cup.png");
                                float x = j*16.0f, y = i*16.0f;
                                cup = Physics::AABB(ZMath::Vec2D(x, y), ZMath::Vec2D(x + 16.0f, y + 16.0f));
                                break;
                            }
                        }

                        ImageResize(&image, 16, 16);

                        // Add the information for the sprite
                        // Note we do not need to set x, y as the matrix tells us the positions for us (since we have uniform tiles)
                        grid[i][j].exists = 1;
                        grid[i][j].texture = LoadTextureFromImage(image);
                        grid[i][j].width = 16;
                        grid[i][j].height = 16;

                        UnloadImage(image);

                        // todo add in the RGB part of the parser (this will be done in terms of tint)
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
                ZMath::Vec2D dP = ball.vel * dt;
                float dPSq = dP.magSq();
                float dist;
                bool yAxis;

                Physics::Ray2D ray(ball.pos, ball.dir);

                // ! This will currently not properly detect collisions due to the ordering of the AABBs mattering (due to break)
                for (uint i = 0; i < numWalls; ++i) {
                    if (Physics::raycast(ray, walls[i], dist, yAxis)) {
                        if (dPSq >= dist*dist) {
                            if (yAxis) { ball.vel.x = -ball.vel.x; ball.dir.x = -ball.dir.x; }
                            else { ball.vel.y = -ball.vel.y; ball.dir.y = -ball.dir.y; }
                        }

                        break;
                    }
                }

                // todo test values for this (both the slowing and the lenience for letting it go in)
                if (Physics::raycast(ray, cup, dist, yAxis) && dPSq >= dist*dist) {
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
            void draw(const ZMath::Vec2D &offset) const {
                for (uint i = 0; i < HEIGHT; ++i) {
                    for (uint j = 0; j < WIDTH; ++j) {
                        if (grid[i][j].exists) { DrawTexture(grid[i][j].texture, j*16 + offset.x, i*16 + offset.y, WHITE); }
                    }
                }
            };

            // Unload the Textures
            // todo probs make this into a function instead of the destructor
            ~Stage() {
                for (uint i = 0; i < HEIGHT; ++i) {
                    for (uint j = 0; j < WIDTH; ++j) {
                        if (grid[i][j].exists) { UnloadTexture(grid[i][j].texture); }
                    }
                }
            };
    };
}

#endif // !TRICKSHOT_H
