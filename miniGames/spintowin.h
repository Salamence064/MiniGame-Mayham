#ifndef SPIN_TO_WIN_H
#define SPIN_TO_WIN_H

#include "../utility.h"
#include "../physics.h"

namespace SpinToWin {
    // todo either make 3D prespectived or 2D top-down

    enum Dir {
        UP,
        RIGHT,
        DOWN,
        LEFT
    };

    class Player {
        private:
            ZMath::Vec2D vel = ZMath::Vec2D();

        public:
            ZMath::Vec2D pos;

            Player(const ZMath::Vec2D &pos) : pos(pos) {};

            void update(float dt);

            void draw() const;
    };

    class Stage {
        // * Section labeling
        // --- | --- | ---
        // -1- | -2- | -3-
        // --- | --- | ---
        // --- | --- | ---
        // -4- | -5- | -6-
        // --- | --- | ---
        // --- | --- | ---
        // -7- | -8- | -9-
        // --- | --- | ---

        public:
            static const uint WIDTH = 9;
            static const uint HEIGHT = 9;
            static const uint BLOCK_WIDTH = 3;
            static const uint BLOCK_HEIGHT = 3;

        private:
            Sprite grid[HEIGHT][WIDTH];
            Player player1, player2;

            float bounceBack; // todo test for a good value

        public:
            Stage();

            // player: 0 = P1, 1 = P2
            void move(bool player, Dir dir);

            void update(float dt);

            void draw() const;
    };
}

#endif // !SPIN_TO_WIN_H
