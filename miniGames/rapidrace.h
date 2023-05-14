#ifndef RAPID_RACE_H
#define RAPID_RACE_H

#include "../utility.h"
#include "../physics.h"

namespace RapidRace {
    // todo can check if a boost panel or rock is hit by checking the filepath
    // todo this current system may feel jerky. Check if it needs interpolation or a float value for yOffset (latter would probs be best)
    // todo make yOffset a float and equivalent to y

    struct Player {
        uint x = 2; // 0-4 for the columns.
        float y = 0.0f; // used for accurate graphical representation.
        uint vel = 1; // 1-5. Number of tiles moved forward per 0.5s. Will be interpolated for smoothness.
        uint yOffset = 0; // number of tiles offset from the start.
    };

    // Stage for the rapid race mini-game
    class Stage {
        public:
            const static uint WIDTH = 5;
            const static uint HEIGHT = 151;

        private:
            // Tiles
            Sprite grid[HEIGHT][WIDTH];

            // Players
            Player player1;
            Player player2;

            float dT = 0.0f; // global dt for the mini-game.
            const static float timeStep = 0.5f;

            // 1 = left, 0 = right.
            bool determineAIMove() const;

        public:
            // Multi: 1 = multiplayer, 0 = vs computer opponent.
            Stage(bool multi);

            void movePlayer1(bool left) {
                if (left) { player1.x = player1.x ? player1.x - 1 : 0; }
                else { player1.x = player1.x < 4 ? player1.x + 1 : 4; }
            };

            void movePlayer2(bool left) {
                if (left) { player2.x = player2.x ? player2.x - 1 : 0; }
                else { player2.x = player2.x < 4 ? player2.x + 1 : 4; }
            };

            // Return 0 if no winner, 1 if P1 wins, 2 if P2 wins, or a 3 in the case of a draw.
            int update(float dt) {
                dT += dt;

                if (dT >= timeStep) {
                    // ! yeahhhh... definitely make this better by just making yOffset non-existant and basing it on y

                    player1.yOffset += player1.vel;
                    player2.yOffset += player2.vel;

                    player1.y = player1.yOffset * 16.0f;
                    player2.y = player2.yOffset * 16.0f;

                    if (player1.yOffset >= 150) {
                        if (player1.yOffset == player2.yOffset) { return 3; }
                        if (player1.yOffset > player2.yOffset) { return 1; }
                    }

                    if (player2.yOffset >= 150) { return 2; }

                    dT -= timeStep;
                    return 0;
                }

                player1.y += (float) player1.vel * 16.0f * (dt/timeStep);
                player2.y += (float) player2.vel * 16.0f * (dt/timeStep);

                if (player1.y >= 2400.0f) {
                    if (ZMath::compare(player1.y, player2.y)) { return 3; }
                    if (player1.y > player2.y) { return 1; }
                }

                if (player2.y >= 2400.0f) { return 2; }

                return 0;
            };

            void draw() const;
    };
}

#endif // !RAPID_RACE_H
