#ifndef UTILITY_H
#define UTILITY_H

// todo maybe add coins in each mini-game that would allow for the player to buy cosmetics from the main menu

#include "raylib.h" // for graphics
#include <string>
typedef unsigned int uint;

struct Sprite {
    bool exists;

    Texture2D texture;
    uint width;
    uint height;
};

#endif // !UTILITY_H
