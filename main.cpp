// ? Main file to manage menus, graphics, and string together mini-games.

#include "trickshot.h"

int main() {
    // Initialization
    static const int screenWidth = 1800;
    static const int screenHeight = 900;

    InitWindow(screenWidth, screenHeight, "Mini-Game Mayham");


    int currStage = 0;
    TrickShot::Stage stages[5];

    stages[0].init("assets/maps/map1.map");
    stages[1].init("assets/maps/map2.map");
    stages[2].init("assets/maps/map3.map");
    stages[3].init("assets/maps/map4.map");
    stages[4].init("assets/maps/map5.map");

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
            flag = stages[currStage].update(timeStep);

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
                if (dP.magSq() >= 550.0f) { stages[currStage].shoot(dP); }
            }
        }

        if (stages[currStage].complete && IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            currStage++;
            if (currStage == 5) {
                for (int i = 0; i < 5; ++i) { stages[i].reset(); }
                currStage = 0;
            }
        }

        // * Draw
        BeginDrawing();

            ClearBackground(BLACK);

            stages[currStage].draw();

            DrawFPS(10, 50);

        EndDrawing();

        dt += GetFrameTime();
    }

    CloseWindow();
    return 0;
};
