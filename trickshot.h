#ifndef TRICKSHOT_H
#define TRICKSHOT_H

#include <fstream>
#include <sstream>
#include "raylib.h"
#include "physics.h"

typedef unsigned int uint;

// todo add sequential levels after beating one

// * =======================
// * Trick Shot Backend
// * =======================

namespace TrickShot {
    struct Ball {
        Color color; // color of the golf ball.
        Physics::Circle hitbox; // Circle representing the ball.
        ZMath::Vec2D vel; // ball's velocity in terms of pixels.
        ZMath::Vec2D prevPos; // ball's previous position.
        float linearDamping = 0.98f; // friction applied to the ball.
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
            char** grid; // grid for drawing the sprites

            Ball ball; // The ball the player shoots.
            Physics::Circle hole; // Circle representing the hole. This should lay in one tile.

            // textures
            Texture2D wallText;
            Texture2D panelText;
            Texture2D sandText;
            Texture2D waterText;

            // colliders
            Physics::AABB* tiles; // special tiles.
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

            uint strokes = 1; // number of strokes the player has taken
            bool canHit = 0; // used to determine if the ball can hit the hole

        public:
            Stage() {};

            // * Do not allow for the creation of the Stage objects through copy constructors or copy assignment operators.

            Stage(Stage const &stage) { throw std::runtime_error("TrickShot::Stage objects CANNOT be created from another TrickShot::Stage object."); };
            
            Stage& operator = (Stage const &Stage) { throw std::runtime_error("TrickShot::Stage objcts CANNOT be assigned or reassigned with '='."); };

            // Initialize a stage for the trickshot minigame.
            // This will randomly select one of the possible stages for the minigame.
            void init() {
                // Set the textures
                Image image1 = LoadImage("assets/wall.png");
                Image image2 = LoadImage("assets/boostPanel.png");
                Image image3 = LoadImage("assets/sand.png");
                Image image4 = LoadImage("assets/water.png");

                ImageResize(&image1, 16, 16);
                ImageResize(&image2, 16, 16);
                ImageResize(&image3, 16, 16);
                ImageResize(&image4, 16, 16);

                wallText = LoadTextureFromImage(image1);
                panelText = LoadTextureFromImage(image2);
                sandText = LoadTextureFromImage(image3);
                waterText = LoadTextureFromImage(image4);

                UnloadImage(image1);
                UnloadImage(image2);
                UnloadImage(image3);
                UnloadImage(image4);

                // Set up the rest of the stage
                std::ifstream f("assets/maps/map5.map");
                std::string line;

                getline(f, line);
                width = std::stoi(line);

                getline(f, line);
                height = std::stoi(line);

                offset = ZMath::Vec2D((1800 - 16*width)/2, (900 - 16*height)/2);
                grid = new char*[height];
                for (uint i = 0; i < height; ++i) { grid[i] = new char[width]; }

                getline(f, line);
                numWalls = std::stoi(line);

                getline(f, line);
                numPanels = std::stoi(line);

                getline(f, line);
                numSand = std::stoi(line);

                getline(f, line);
                numWater = std::stoi(line);

                panelOffset = numWalls + numPanels;
                sandOffset = numWalls + numPanels + numSand;
                waterOffset = numWalls + numPanels + numSand + numWater;

                tiles = new Physics::AABB[waterOffset];

                for (uint i = 0; i < height; ++i) {
                    getline(f, line);
                    for (uint j = 0; j < width; ++j) {
                        if (line[j] == 'b') {
                            ball = {WHITE, Physics::Circle(offset + ZMath::Vec2D(j*16 + 8.0f, i*16 + 8.0f), 8.0f), ZMath::Vec2D(), ball.hitbox.c};
                            startingPos = ball.hitbox.c;
                            continue;
                        }

                        if (line[j] == 'h') {
                            hole = Physics::Circle(offset + ZMath::Vec2D(j*16 + 8.0f, i*16 + 8.0f), 8.0f);
                            continue;
                        }

                        grid[i][j] = line[j];
                    }
                }

                size_t i1, i2, i3;
                for (uint i = 0; i < waterOffset; ++i) {
                    getline(f, line);

                    i1 = line.find(",");
                    i2 = line.find("|");
                    i3 = line.find(",", i2);

                    tiles[i] = Physics::AABB(
                        offset + ZMath::Vec2D(std::stof(line.substr(0, i1)), std::stof(line.substr(i1 + 1, i2))),
                        offset + ZMath::Vec2D(std::stof(line.substr(i2 + 1, i3)), std::stof(line.substr(i3 + 1)))
                    );
                }
            };

            /**
             * @brief Shoot the ball in the direction determined by the player releasing the mouse.
             * 
             * @param dm Change in the position of the mouse since it was pressed down.
             */
            inline void shoot(const ZMath::Vec2D &dm) {
                ball.vel.set(dm);
                strokes++;
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
                Physics::Line2D dP(ball.prevPos, ball.hitbox.c);

                // wall collisions
                for (uint i = 0; i < numWalls; ++i) {
                    ZMath::Vec2D min = tiles[i].getMin(), max = tiles[i].getMax();

                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i], n)) {        
                        if (!ZMath::compare(n.x, 0) && !ZMath::compare(n.y, 0)) { // corner collision
                            ball.vel = -ball.vel;
                            ball.hitbox.c += ball.vel * dt;

                        } else if (!ZMath::compare(n.x, 0)) {
                            ball.vel.x = -ball.vel.x;
                            ball.hitbox.c.x += ball.vel.x * dt; // apply the velocity a second time this iteration to ensure it escapes the wall.

                        } else if (!ZMath::compare(n.y, 0)) {
                            ball.vel.y = -ball.vel.y;
                            ball.hitbox.c.y += ball.vel.y * dt; // apply the velocity a second time this iteration to ensure it escapes the wall.

                        } else {
                            if ((min.x > ball.prevPos.x && ball.hitbox.c.x > min.x) || (max.x < ball.prevPos.x && ball.hitbox.c.x < max.x)) {
                                ball.vel.x = -ball.vel.x;
                                ball.hitbox.c.x += ball.vel.x * dt;

                            } else {
                                ball.vel.y = -ball.vel.y;
                                ball.hitbox.c.y += ball.vel.y * dt;
                            }
                        }

                    } else if ((ball.prevPos.x - tiles[i].pos.x) * (ball.hitbox.c.x - tiles[i].pos.x) < 0.0f &&
                                ball.hitbox.c.y >= min.y && max.y >= ball.hitbox.c.y) {
                        ball.vel.x = -ball.vel.x;
                        ball.hitbox.c.x += ball.vel.x * dt;

                    } else if ((ball.prevPos.y - tiles[i].pos.y) * (ball.hitbox.c.y - tiles[i].pos.y) < 0.0f &&
                                ball.hitbox.c.x >= min.x && max.x >= ball.hitbox.c.x) {
                        ball.vel.y = -ball.vel.y;
                        ball.hitbox.c.y += ball.vel.y * dt;
                    }
                }

                // boost panels
                for (uint i = numWalls; i < panelOffset; ++i) {
                    if (Physics::CircleAndAABB(ball.hitbox, tiles[i])) {
                        if (ball.vel.magSq() < 1000000.0f) { ball.vel *= 1.1f; }
                    }
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
                    if (ball.vel.magSq() <= 20000.0f) { complete = 1; return 0; }
                    ball.vel *= 0.35f;
                    canHit = 0;
                }

                ball.prevPos = ball.hitbox.c;
                ball.hitbox.c += ball.vel * dt;
                ball.vel *= ball.linearDamping;

                if (ball.vel.magSq() <= 100.0f) {
                    ball.vel.zero();
                    return 1;
                }

                return 0;
            };

            // Draw the tiles associated with the stage.
            inline void draw() const {
                DrawRectangle(offset.x, offset.y, 16.0f*width, 16.0f*height, {0, 145, 50, 255});
                
                for (uint i = 0; i < height; ++i) {
                    for (uint j = 0; j < width; ++j) {
                        switch(grid[i][j]) {
                            case 'w': { DrawTexture(wallText, offset.x + j*16, offset.y + i*16, WHITE); break; }
                            case 'B': { DrawTexture(panelText, offset.x + j*16, offset.y + i*16, WHITE); break; }
                            case 's': { DrawTexture(sandText, offset.x + j*16, offset.y + i*16, WHITE); break; }
                            case 'W': { DrawTexture(waterText, offset.x + j*16, offset.y + i*16, WHITE); break; }
                        }
                    }
                }

                DrawCircle(hole.c.x, hole.c.y, hole.r, BLACK);
                DrawCircle(ball.hitbox.c.x, ball.hitbox.c.y, ball.hitbox.r, ball.color);

                if (complete) {
                    std::ostringstream sout;
                    if (strokes == 2) { sout << "Hole in One!"; }
                    else { sout << "You made it in " << (strokes - 1) << " strokes!"; }
                    int textWidth = MeasureText(sout.str().c_str(), 50);

                    DrawText(sout.str().c_str(), (1800 - textWidth)/2, 425, 50, WHITE);

                } else {
                    std::ostringstream sout;
                    sout << "Stroke: " << strokes;
                    DrawText(sout.str().c_str(), 10, 10, 30, WHITE);
                }
            };

            inline void reset() {
                ball.hitbox.c = startingPos;
                ball.prevPos = startingPos;
                ball.vel.zero();

                strokes = 1;
                canHit = 0;
                complete = 0;
            };

            ~Stage() {
                // free the memory
                for (uint i = 0; i < height; ++i) { delete[] grid[i]; }
                delete[] grid;
                delete[] tiles;

                // unload the textures from the VRAM
                UnloadTexture(wallText);
                UnloadTexture(panelText);
                UnloadTexture(sandText);
                UnloadTexture(waterText);
            };
    };
}

#endif // !TRICKSHOT_H
