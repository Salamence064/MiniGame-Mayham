// ? Main file to manage menus, graphics, and string together mini-games.

// todo in CFlags line in makefile with -std=c++20 add -O3 and remove -Wall for the finished version (add -s too)
// todo remove -g and -O0, too

#include "miniGames/trickshot.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

int main() {
    // Initialization
    static const int screenWidth = 1800;
    static const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Mini-Game Mayham");

    // ! Could use templated list or structs to make this storage a bit better
    TrickShot::Stage trickShotStage = TrickShot::Stage();
    ZMath::Vec2D trickshotOffset(740.0f, 290.0f);

    #if defined(PLATFORM_WEB)
        emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
    #else

        // Main game loop
        while (!WindowShouldClose()) {
            // * Update
    


            // * Draw
            BeginDrawing();

                ClearBackground(BLACK);

                trickShotStage.draw(trickshotOffset);

                DrawFPS(10, 10);

            EndDrawing();
        }

    #endif

    CloseWindow();
    return 0;
};
