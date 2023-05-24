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
        Texture2D texture; // texture for the ball's sprite
        ZMath::Vec2D pos; // ball's position in terms of pixels
        ZMath::Vec2D vel; // ball's velocity in terms of pixels
        ZMath::Vec2D dir; // normalized direction of the ball -- cached for efficiency
        float linearDamping = 0.98f; // friction applied to the ball
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
            static const uint WIDTH = 50; // width of the board
            static const uint HEIGHT = 50; // height of the board

            bool complete = 0; // has the stage been completed

        private:
            // todo also do not store the ball's pos in the grid and draw it separately
            Sprite grid[HEIGHT][WIDTH];
            
            Ball ball; // The ball the player shoots.
            Physics::AABB cup; // AABB representing the cup. This should lay in one tile.

            Physics::AABB* walls; // List of walls the player can collide with.
            uint numWalls = 0; // number of walls

        public:          
            Stage() {};

            // * Do not allow for the creation of the Stage objects through copy constructors or copy assignment operators.

            Stage(Stage const &stage) { throw std::runtime_error("TrickShot::Stage objects CANNOT be created from another TrickShot::Stage object."); };
            
            Stage& operator = (Stage const &Stage) { throw std::runtime_error("TrickShot::Stage objcts CANNOT be assigned or reassigned with '='."); };

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

            // Initialize a stage for the trickshot minigame.
            // This will randomly select one of the possible stages for the minigame.
            void init() {
                std::ifstream f("miniGames/resources/trickshot/maps/map1.map");

                std::string line;

                for (uint i = 0; i < HEIGHT; ++i) {
                    getline(f, line);

                    Image image;

                    for (uint j = 0; j < WIDTH; ++j) {
                        switch (line[j]) {
                            case 'w': {
                                image = LoadImage("miniGames/resources/trickshot/ball.png");
                                break;
                            }

                            case 'b': {
                                image = LoadImage("miniGames/resources/trickshot/ball.png");
                                ImageResize(&image, 16, 16);
                                ball = {LoadTextureFromImage(image), ZMath::Vec2D(j*16.0f, i*16.0f), ZMath::Vec2D(), ZMath::Vec2D()};
                                grid[i][j].exists = 0;
                                continue;
                            }

                            case 'c': {
                                image = LoadImage("miniGames/resources/trickshot/cup.png");
                                float x = j*16.0f, y = i*16.0f;
                                cup = Physics::AABB(ZMath::Vec2D(x, y), ZMath::Vec2D(x + 16.0f, y + 16.0f));
                                break;
                            }

                            default: {
                                grid[i][j].exists = 0;
                                continue;
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
            void shoot(const ZMath::Vec2D &dm) {
                ball.vel.set(dm);
                ball.dir.set(ball.vel.normalize());

                // todo update with better values once testing can be done
            };

            /**
             * @brief Update the position of the ball while its velocity is not 0.
             * 
             * @param dt The time step passed. This should be standardized by the physics engine for determinism.
             * @return 0 while the magnitude of the velocity is greater than the cut-off and 1 once its magnitude reaches that cut-off.
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

                if (ball.vel.magSq() <= 100.0f) {
                    ball.vel.zero();
                    return 1;
                }

                return 0;
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

                DrawTexture(ball.texture, offset.x + ball.pos.x, offset.y + ball.pos.y, WHITE);
            };

            // Unload the Textures
            // todo probs make this into a function instead of the destructor
            ~Stage() {
                for (uint i = 0; i < HEIGHT; ++i) {
                    for (uint j = 0; j < WIDTH; ++j) {
                        if (grid[i][j].exists) { UnloadTexture(grid[i][j].texture); }
                    }
                }

                UnloadTexture(ball.texture);
            };
    };
}

#endif // !TRICKSHOT_H
