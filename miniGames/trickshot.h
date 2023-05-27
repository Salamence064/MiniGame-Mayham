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
    // todo make a sprite atlas for the game instead of having them separate like it is currently

    struct Ball {
        Color color; // color of the golf ball.
        Physics::Circle hitbox; // Circle representing the ball.
        ZMath::Vec2D vel; // ball's velocity in terms of pixels
        float linearDamping = 0.98f; // friction applied to the ball
    };

    struct Tile {
        Texture2D text; // Texture of the tile.
        Physics::AABB collider; // AABB collider of the tile.
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
            uint width = 50; // width of the board
            uint height = 50; // height of the board

            bool complete = 0; // has the stage been completed

        private:
            Ball ball; // The ball the player shoots.
            Physics::Circle hole; // Circle representing the hole. This should lay in one tile.

            Tile* tiles; // special tiles.
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
                width = std::stoi(line);

                getline(f, line);
                height = std::stoi(line);

                getline(f, line);
                numWalls = std::stoi(line);

                getline(f, line);
                numPanels = std::stoi(line);

                getline(f, line);
                numSand = std::stoi(line);

                getline(f, line);
                numWater = std::stoi(line);
                tiles = new Tile[numWalls + numPanels + numSand + numWater];

                panelOffset = numWalls + numPanels;
                sandOffset = numWalls + numPanels + numSand;
                waterOffset = numWalls + numPanels + numSand + numWater;

                uint temp1 = 0, temp2 = 0, temp3 = 0, temp4 = 0; // temp values for counting

                Image image1 = LoadImage("miniGames/assets/trickshot/border1.png");
                Image image2 = LoadImage("miniGames/assets/trickshot/border2.png");

                ImageResize(&image1, 16, 800);
                ImageResize(&image2, 768, 16);

                tiles[temp1++] = {LoadTextureFromImage(image1), Physics::AABB(offset + ZMath::Vec2D(), offset + ZMath::Vec2D(16, 800))};
                tiles[temp1++] = {LoadTextureFromImage(image2), Physics::AABB(offset + ZMath::Vec2D(16, 0), offset + ZMath::Vec2D(800, 16))};
                tiles[temp1++] = {LoadTextureFromImage(image1), Physics::AABB(offset + ZMath::Vec2D(784, 0), offset + ZMath::Vec2D(800, 800))};
                tiles[temp1++] = {LoadTextureFromImage(image2), Physics::AABB(offset + ZMath::Vec2D(16, 784), offset + ZMath::Vec2D(800, 800))};

                UnloadImage(image1);
                UnloadImage(image2);

                for (uint i = 0; i < height; ++i) {
                    getline(f, line);

                    Image image;

                    for (uint j = 0; j < width; ++j) {
                        switch (line[j]) {
                            case 'w': { // wall
                                image = LoadImage("miniGames/assets/trickshot/wall.png");
                                ImageResize(&image, 16, 16);

                                float x = j*16.0f, y = i*16.0f;
                                tiles[temp1++] = {
                                     LoadTextureFromImage(image),
                                     Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f))
                                };
                                
                                UnloadImage(image);
                                break;
                            }

                            case 'b': { // ball
                                ball = {WHITE, Physics::Circle((offset + ZMath::Vec2D(j*16.0f + 8.0f, i*16.0f + 8.0f)), 8.0f), ZMath::Vec2D()};
                                startingPos = ball.hitbox.c;
                                break;
                            }

                            case 'h': { // hole
                                hole = Physics::Circle(offset + ZMath::Vec2D(j*16.0f + 8.0f, i*16.0f + 8.0f), 8.0f);
                                break;
                            }

                            case 'B': { // boost panel
                                image = LoadImage("miniGames/assets/trickshot/boostPanel.png");
                                ImageResize(&image, 16, 16);

                                float x = j*16.0f, y = i*16.0f;
                                tiles[numWalls + temp2++] = {
                                    LoadTextureFromImage(image),
                                    Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f))
                                };

                                UnloadImage(image);
                                break;
                            }

                            case 's': { // sand
                                image = LoadImage("miniGames/assets/trickshot/sand.png");
                                ImageResize(&image, 16, 16);

                                float x = j*16.0f, y = i*16.0f;
                                tiles[panelOffset + temp3++] = {
                                    LoadTextureFromImage(image),
                                    Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f))
                                };

                                UnloadImage(image);
                                break;
                            }

                            case 'W': { // water
                                image = LoadImage("miniGames/assets/trickshot/water.png");
                                ImageResize(&image, 16, 16);

                                float x = j*16.0f, y = i*16.0f;
                                tiles[sandOffset + temp4++] = {
                                    LoadTextureFromImage(image),
                                    Physics::AABB(offset + ZMath::Vec2D(x, y), offset + ZMath::Vec2D(x + 16.0f, y + 16.0f))
                                };

                                UnloadImage(image);
                                break;
                            }
                        }
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
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i].collider, n)) {                        
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

                // boost panels
                for (uint i = numWalls; i < panelOffset; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i].collider)) { ball.vel *= 1.1f; }
                }

                // sand
                for (uint i = panelOffset; i < sandOffset; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i].collider)) { ball.vel *= 0.965f; }
                }

                // water
                for (uint i = sandOffset; i < waterOffset; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i].collider)) {
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
                DrawRectangle(offset.x, offset.y, 16.0f*width, 16.0f*height, {0, 145, 50, 255});
                
                ZMath::Vec2D v;
                for (uint i = 0; i < waterOffset; ++i) {
                    v = tiles[i].collider.getMin();
                    DrawTexture(tiles[i].text, v.x, v.y, WHITE);

                    // std::ostringstream sout;
                    // sout << v.x << ", " << v.y;

                    // DrawText(sout.str().c_str(), v.x, v.y, 16, WHITE);
                }

                DrawCircle(hole.c.x, hole.c.y, hole.r, BLACK);
                DrawCircle(ball.hitbox.c.x, ball.hitbox.c.y, ball.hitbox.r, ball.color);
            };

            // Unload the Textures
            // todo probs make this into a function instead of the destructor
            ~Stage() { for (uint i = 0; i < waterOffset; ++i) { UnloadTexture(tiles[i].text); }};
    };
}

#endif // !TRICKSHOT_H
