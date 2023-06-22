// ? Main file to manage menus, graphics, and string together mini-games.

#include "trickshot.h"

TrickShot::Stage trickShotStage = TrickShot::Stage();

int main() {
    // Initialization
    static const int screenWidth = 1800;
    static const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Mini-Game Mayham");

    trickShotStage.init();

    // used to track delta mouse
    ZMath::Vec2D startMPos;

    // dt stuff
    float dt = 0.0f;
    float timeStep = 0.0167f;

    // flag bool used to determine if the player can input
    bool flag = 1;

    // Main game loop
    while (!WindowShouldClose()) {

        // * Update
        while(dt >= timeStep) {
            // update the trickshot stage
            flag = trickShotStage.update(timeStep);

            dt -= timeStep;
        }

        // check for mouse dragging
        if (flag) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                startMPos.x = GetMouseX();
                startMPos.y = GetMouseY();
            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
                ZMath::Vec2D dP = ZMath::Vec2D(startMPos.x - GetMouseX(), startMPos.y - GetMouseY());
                if (dP.magSq() >= 550.0f) { trickShotStage.shoot(dP); }
            }
        }

        if (trickShotStage.complete && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) { trickShotStage.reset(); }

        // * Draw
        BeginDrawing();

            ClearBackground(BLACK);

            trickShotStage.draw();

            DrawFPS(10, 50);

        EndDrawing();

        dt += GetFrameTime();
    }

    CloseWindow();
    return 0;
};
