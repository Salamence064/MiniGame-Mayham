#ifndef ITS_TIME_TO_DUEL_H
#define ITS_TIME_TO_DUEL_H

#include "raylib.h"
#include "../physics.h"

typedef unsigned int uint;

// todo update to model the players' hitboxes with ellipses

namespace Duel {
    enum Event {
        PLAYER1_SWORD,
        PLAYER2_SWORD,
        PLAYER1_BOW,
        PLAYER2_BOW,
        PLAYER1_MOVE,
        PLAYER2_MOVE
    };

    class Player {
        public:
            ZMath::Vec2D pos;
            ZMath::Vec2D vel;

            Physics::Circle hitbox; // Circle representing the hitbox of the player.

            //std::string filepath; // Filepath for the sprite of the player.
            int hp; // todo test for good hp values
        
            Player(const ZMath::Vec2D &pos);

            void attack(bool sword) const {
                if (sword) {
                    // ! Throw out the sword hurtbox
                    return;
                }

                // ! Shoot out an arrow
            };

            void update(float dt) {

            };

            void draw() const;
    };

    class Stage {
        public:
            // width = numCols
            // height = numRows
            static const uint WIDTH = 50; // width of the board
            static const uint HEIGHT = 50; // height of the board

        private:
            //Sprite grid[HEIGHT][WIDTH];
            Player player1;
            Player player2;
            
        public:
            Stage();

            // Event handler for this game.
            void onEvent(Event e, const ZMath::Vec2D &vel = ZMath::Vec2D(0, 0));

            // Update the players according to their actions.
            void update(float dt);

            // Draw the tiles associated with the stage.
            void draw() const;
    };
}

#endif // !ITS_TIME_TO_DUEL_H
