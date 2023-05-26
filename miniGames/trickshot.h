#ifndef TRICKSHOT_H
#define TRICKSHOT_H

#include <fstream>
#include <sstream>
#include "../utility.h"
#include "../physics.h"

// * =======================
// * Trick Shot Backend
// * =======================

namespace TrickShot {
    struct Ball {
        Color color; // color of the golf ball.
        Physics::Circle hitbox; // Circle representing the ball.
        ZMath::Vec2D vel; // ball's velocity in terms of pixels
        float linearDamping = 0.98f; // friction applied to the ball
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
        // todo remove the grid system
            Sprite grid[HEIGHT][WIDTH];
            
            Ball ball; // The ball the player shoots.
            Physics::Circle hole; // Circle representing the hole. This should lay in one tile.

            Physics::AABB* tiles; // special tiles listed as AABBs.
            uint numWalls = 0; // number of walls.
            uint numPanels = 0; // number of boost panels.
            uint numSand = 0; // number of sand tiles.
            uint numWater = 0; // number of water tiles.

            // offsets
            uint panelOffset;
            uint sandOffset;
            uint waterOffset;

            ZMath::Vec2D startingPos; // starting position of the ball
            ZMath::Vec2D offset; // offset to center the stage in the screen

            bool canHit = 0; // used to determine if the ball can hit the hole

        public:          
            Stage() {};

            // * Do not allow for the creation of the Stage objects through copy constructors or copy assignment operators.

            Stage(Stage const &stage) { throw std::runtime_error("TrickShot::Stage objects CANNOT be created from another TrickShot::Stage object."); };
            
            Stage& operator = (Stage const &Stage) { throw std::runtime_error("TrickShot::Stage objcts CANNOT be assigned or reassigned with '='."); };

            /** 
             * Symbol Legend:
             * 
             * space = nothing
             * w = wall
             * b = ball
             * h = hole
             * B = boost panel
             * s = sand
             * W = water
             * 
             * Each can be followed by an RGB to shade it a different color in the format [r],[b],[g]
             */

            // Initialize a stage for the trickshot minigame.
            // This will randomly select one of the possible stages for the minigame.
            void init(ZMath::Vec2D const &offset) {
                this->offset = offset;

                std::ifstream f("miniGames/assets/trickshot/maps/map1.map");
                std::string line;

                getline(f, line);
                numWalls = std::stoi(line);

                getline(f, line);
                numPanels = std::stoi(line);

                getline(f, line);
                numSand = std::stoi(line);

                getline(f, line);
                numWater = std::stoi(line);
                tiles = new Physics::AABB[numWalls + numPanels + numSand + numWater];

                panelOffset = numWalls + numPanels;
                sandOffset = numWalls + numPanels + numSand;
                waterOffset = numWalls + numPanels + numSand + numWater;

                uint temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0; // temp values for counting

                tiles[temp1++] = Physics::AABB(offset + ZMath::Vec2D(), offset + ZMath::Vec2D(WIDTH*16.0f, 16.0f));
                tiles[temp1++] = Physics::AABB(offset + ZMath::Vec2D(), offset + ZMath::Vec2D(16.0f, HEIGHT*16.0f));
                tiles[temp1++] = Physics::AABB(offset + ZMath::Vec2D(0.0f, HEIGHT*16.0f - 16.0f), offset + ZMath::Vec2D(WIDTH*16.0f, HEIGHT*16.0f));
                tiles[temp1++] = Physics::AABB(offset + ZMath::Vec2D(WIDTH*16.0f - 16.0f, 0.0f), offset + ZMath::Vec2D(WIDTH*16.0f, HEIGHT*16.0f));

                for (uint i = 0; i < HEIGHT; ++i) {
                    getline(f, line);

                    Image image;

                    for (uint j = 0; j < WIDTH; ++j) {
                        switch (line[j]) {
                            // todo make a border sprite for the borders which is just the sprites meshed together
                            // todo make a sprite atlas for the game instead of having them separate like it is currently

                            case 'w': { // wall
                                image = LoadImage("miniGames/assets/trickshot/wall.png");

                                if (i && j && i != HEIGHT - 1 && j != WIDTH - 1) {
                                    float x = j*16.0f, y = i*16.0f;
                                    tiles[temp1++] = Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f));
                                }

                                ImageResize(&image, 16, 16);

                                // Add the information for the sprite
                                grid[i][j].exists = 1;
                                grid[i][j].texture = LoadTextureFromImage(image);
                                grid[i][j].width = 16;
                                grid[i][j].height = 16;

                                UnloadImage(image);
                                break;
                            }

                            case 'b': { // ball
                                ball = {WHITE, Physics::Circle((offset + ZMath::Vec2D(j*16.0f + 8.0f, i*16.0f + 8.0f)), 8.0f), ZMath::Vec2D()};
                                startingPos = ball.hitbox.c;
                                grid[i][j].exists = 0;
                                break;
                            }

                            case 'h': { // hole
                                hole = Physics::Circle(offset + ZMath::Vec2D(j*16.0f + 8.0f, i*16.0f + 8.0f), 8.0f);
                                grid[i][j].exists = 0;
                                break;
                            }

                            case 'B': { // boost panel
                                float x = j*16.0f, y = i*16.0f;
                                tiles[numWalls + temp2++] = Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f));

                                image = LoadImage("miniGames/assets/trickshot/boostPanel.png");
                                ImageResize(&image, 16, 16);

                                // Add the information for the sprite
                                grid[i][j].exists = 1;
                                grid[i][j].texture = LoadTextureFromImage(image);
                                grid[i][j].width = 16;
                                grid[i][j].height = 16;

                                UnloadImage(image);
                                break;
                            }

                            case 's': { // sand
                                float x = j*16.0f, y = i*16.0f;
                                tiles[numWalls + numPanels + temp3++] = Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f));

                                image = LoadImage("miniGames/assets/trickshot/sand.png");
                                ImageResize(&image, 16, 16);

                                // Add the information for the sprite
                                grid[i][j].exists = 1;
                                grid[i][j].texture = LoadTextureFromImage(image);
                                grid[i][j].width = 16;
                                grid[i][j].height = 16;

                                UnloadImage(image);
                                break;
                            }

                            case 'W': { // water
                                float x = j*16.0f, y = i*16.0f;
                                tiles[numWalls + numPanels + numSand + temp4++] = Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f));

                                image = LoadImage("miniGames/assets/trickshot/water.png");
                                ImageResize(&image, 16, 16);

                                // Add the information for the sprite
                                grid[i][j].exists = 1;
                                grid[i][j].texture = LoadTextureFromImage(image);
                                grid[i][j].width = 16;
                                grid[i][j].height = 16;

                                UnloadImage(image);
                                break;
                            }

                            default: {
                                grid[i][j].exists = 0;
                                break;
                            }
                        }

                        // todo add in the RGB part of the parser (this will be done in terms of tint)
                    }
                }
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

                // wall collisions
                for (uint i = 0; i < numWalls; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i], n)) {                        
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

                // todo make offset variables

                // boost panels
                for (uint i = numWalls; i < panelOffset; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i])) { ball.vel *= 1.1f; }
                }

                // sand
                for (uint i = panelOffset; i < sandOffset; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i])) { ball.vel *= 0.965f; }
                }

                // water
                for (uint i = sandOffset; i < waterOffset; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i])) {
                        ball.hitbox.c = startingPos;
                        ball.vel.zero();
                        return 1;
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
            // todo make this func
            void drawShootingUI(ZMath::Vec2D const &mousePos) const;

            // Draw the tiles associated with the stage.
            inline void draw() const {
                for (uint i = 0; i < HEIGHT; ++i) {
                    for (uint j = 0; j < WIDTH; ++j) {
                        if (grid[i][j].exists) { DrawTexture(grid[i][j].texture, j*16 + offset.x, i*16 + offset.y, WHITE); }
                        else { DrawRectangle(j*16.0f + offset.x, i*16.0f + offset.y, 16.0f, 16.0f, {0, 145, 50, 255}); }
                    }
                }

                DrawCircle(hole.c.x, hole.c.y, hole.r, BLACK);
                DrawCircle(ball.hitbox.c.x, ball.hitbox.c.y, ball.hitbox.r, ball.color);

                // * Debug stuff
                // std::ostringstream sout;
                // sout << "Ball.c: " << ball.hitbox.c.x << ", " << ball.hitbox.c.y << "\nBall.r: " << ball.hitbox.r;
                // DrawText(sout.str().c_str(), 10, 50, 30, WHITE);

                // std::ostringstream sout1;
                // sout1 << "Hole.c: " << hole.c.x << ", " << hole.c.y << "\nHole.r: " << hole.r;
                // DrawText(sout1.str().c_str(), 1400, 10, 30, WHITE);
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
