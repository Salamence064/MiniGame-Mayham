#ifndef UTILITY_H
#define UTILITY_H

#include <string>
typedef unsigned int uint;

struct Sprite {
    std::string filepath;
    uint x = 0;
    uint y = 0;
    static const uint width = 16;
    static const uint height = 16;
};

#endif // !UTILITY_H
