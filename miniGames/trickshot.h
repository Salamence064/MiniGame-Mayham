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
        Physics::Circle hitbox; // Circle representing the ball.
        ZMath::Vec2D vel; // ball's velocity in terms of pixels
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
            Physics::Circle hole; // AABB representing the hole. This should lay in one tile.

            Physics::AABB* walls; // List of walls the player can collide with.
            uint numWalls = 0; // number of walls

            ZMath::Vec2D offset; // offset to center the stage in the screen

            bool canHit = 0; // used to determine if the ball can hit the hole

        public:          
            Stage() {};

            // * Do not allow for the creation of the Stage objects through copy constructors or copy assignment operators.

            Stage(Stage const &stage) { throw std::runtime_error("TrickShot::Stage objects CANNOT be created from another TrickShot::Stage object."); };
            
            Stage& operator = (Stage const &Stage) { throw std::runtime_error("TrickShot::Stage objcts CANNOT be assigned or reassigned with '='."); };

            // todo add one more block type -- probs a block that could slow down the ball
            // todo maybe one that could speed it up again, too
            // todo maybe a water hazard type block, too, that would force a restart
            /** 
             * Symbol Legend:
             * 
             * space = nothing
             * w = wall
             * b = ball
             * h = hole
             * 
             * Each can be followed by an RGB to shade it a different color in the format [r],[b],[g]
             */

            // Initialize a stage for the trickshot minigame.
            // This will randomly select one of the possible stages for the minigame.
            void init(ZMath::Vec2D const &offset) {
                this->offset = offset;

                std::ifstream f("miniGames/resources/trickshot/maps/map1.map");

                std::string line;

                getline(f, line);
                numWalls = std::stoi(line);
                walls = new Physics::AABB[numWalls];
                int temp = 0; // temp index for the walls

                for (uint i = 0; i < HEIGHT; ++i) {
                    getline(f, line);

                    Image image;

                    for (uint j = 0; j < WIDTH; ++j) {
                        switch (line[j]) {
                            case 'w': {
                                image = LoadImage("miniGames/resources/trickshot/ball.png");
                                if (i && j && i != HEIGHT - 1 && j != WIDTH - 1) {
                                    float x = j*16.0f, y = i*16.0f;
                                    walls[temp++] = Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f));
                                }

                                break;
                            }

                            case 'b': {
                                image = LoadImage("miniGames/resources/trickshot/ball.png");
                                ImageResize(&image, 16, 16);
                                ball = {LoadTextureFromImage(image), Physics::Circle((offset + ZMath::Vec2D(j*16.0f + 8.0f, i*16.0f + 8.0f)), 8.0f), ZMath::Vec2D()};
                                grid[i][j].exists = 0;
                                continue;
                            }

                            case 'h': {
                                image = LoadImage("miniGames/resources/trickshot/hole.png");
                                hole = Physics::Circle(offset + ZMath::Vec2D(j*16.0f + 8.0f, i*16.0f + 8.0f), 8.0f);
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

                walls[temp++] = Physics::AABB(offset + ZMath::Vec2D(), offset + ZMath::Vec2D(WIDTH*16.0f, 0.0f));
                walls[temp++] = Physics::AABB(offset + ZMath::Vec2D(), offset + ZMath::Vec2D(0.0f, HEIGHT*16.0f));
                walls[temp++] = Physics::AABB(offset + ZMath::Vec2D(0.0f, HEIGHT*16.0f), offset + ZMath::Vec2D(WIDTH*16.0f, HEIGHT*16.0f));
                walls[temp++] = Physics::AABB(offset + ZMath::Vec2D(WIDTH*16.0f, 0.0f), offset + ZMath::Vec2D(WIDTH*16.0f, HEIGHT*16.0f));
            };

            /**
             * @brief Shoot the ball in the direction determined by the player releasing the mouse.
             * 
             * @param dm Change in the position of the mouse since it was pressed down.
             */
            inline void shoot(const ZMath::Vec2D &dm) {
                ball.vel.set(dm);
                canHit = 1;
            };

            /**
             * @brief Update the position of the ball while its velocity is not 0.
             * 
             * @param dt The time step passed. This should be standardized by the physics engine for determinism.
             * @return 0 while the magnitude of the velocity is greater than the cut-off and 1 once its magnitude reaches that cut-off.
             */
            bool update(float dt) {
                ZMath::Vec2D n;

                for (uint i = 0; i < numWalls; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, walls[i], n)) {                        
                        if (std::fabs(n.x) > std::fabs(n.y)) {
                            ball.vel.x = -ball.vel.x;
                            ball.hitbox.c.x += ball.vel.x * dt; // apply the velocity a second time this iteration to ensure it escapes the wall.

                        } else {
                            ball.vel.y = -ball.vel.y;
                            ball.hitbox.c.y += ball.vel.y * dt; // apply the velocity a second time this iteration to ensure it escapes the wall.
                        }

                        break;
                    }
                }

                if (canHit && Physics::CircleInCircle(ball.hitbox, hole)) {
                    if (ball.vel.magSq() <= 10000.0f) { complete = 1; return 0; }
                    ball.vel *= 0.45f;
                    canHit = 0;
                }

                ball.hitbox.c += ball.vel * dt;
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
            inline void draw() const {
                for (uint i = 0; i < HEIGHT; ++i) {
                    for (uint j = 0; j < WIDTH; ++j) {
                        if (grid[i][j].exists) { DrawTexture(grid[i][j].texture, j*16 + offset.x, i*16 + offset.y, WHITE); }
                    }
                }

                DrawTexture(ball.texture, ball.hitbox.c.x - 8.0f, ball.hitbox.c.y - 8.0f, WHITE);

                // * =====================
                // * Debug Drawings
                // * =====================

                // DrawCircle(ball.hitbox.c.x, ball.hitbox.c.y, ball.hitbox.r, RED);
                // DrawCircle(hole.c.x, hole.c.y, hole.r, PINK);
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
