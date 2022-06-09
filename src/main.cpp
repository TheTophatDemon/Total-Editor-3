#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include <stdlib.h>
#include <vector>
#include <string>

#include "app.hpp"
#include "assets.hpp"
#include "editor_mode.hpp"
#include "place_mode.hpp"
#include "pick_mode.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char **argv)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Total Editor 3");
	InitAudioDevice();

    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(DARKGRAY));

    AppContext context = {
        .screenWidth = SCREEN_WIDTH,
        .screenHeight = SCREEN_HEIGHT,
        .mouseSensitivity = 0.5f,
        .selectedTexture = Assets::GetTexture("assets/textures/psa.png"),
        .selectedShape = Assets::GetShape("assets/models/shapes/cube.obj")
    };

    Assets::Initialize();

    //Editor modes
    PlaceMode placeMode(&context);
    PickMode pickMode(&context, PickMode::Mode::TEXTURES);
    EditorMode *editorMode = &placeMode;

	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
        if (IsKeyPressed(KEY_TAB)) {
            if (editorMode == &placeMode) {
                editorMode = &pickMode;
            } else {
                editorMode = &placeMode;
            }
        }

        //Update
        editorMode->Update();

        //Draw
		BeginDrawing();
		
		ClearBackground(BLACK);

		editorMode->Draw();
		
        DrawRectangle(2, 2, 128, 32, DARKGRAY);
		DrawFPS(4, 4);

		EndDrawing();
	}
	
	CloseWindow();

	return 0;
}
