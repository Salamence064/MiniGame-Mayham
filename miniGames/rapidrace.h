#ifndef RAPID_RACE_H
#define RAPID_RACE_H

#include "../utility.h"
#include "../physics.h"

namespace RapidRace {
    // todo can check if a boost panel or rock is hit by checking the filepath

    struct Player {
        uint x = 2; // 0-4 for the columns.
        uint vel = 1; // 1-5. Number of tiles moved forward per dt. Will be interpolated for smoothness.
        uint yOffset = 0; // number of tiles offset from the start.
    };

    // Stage for the rapid race mini-game
    class Stage {
        public:
            const static uint WIDTH = 5;
            const static uint HEIGHT = 200;

        private:
            // Tiles
            Sprite grid[HEIGHT][WIDTH];

            // Players
            Player player1;
            Player player2;

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

            void update(float dt);

            void draw() const;
    };
}

#endif // !RAPID_RACE_H
