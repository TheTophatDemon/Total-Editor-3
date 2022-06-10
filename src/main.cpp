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
    SetWindowMinSize(640, 480);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
	InitAudioDevice();
    SetExitKey(KEY_NULL);

    Assets::Initialize();

    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(DARKGRAY));
    GuiSetFont(*Assets::GetFont());
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(LABEL, TEXT_COLOR_FOCUSED, ColorToInt(YELLOW));
    GuiSetStyle(LABEL, TEXT_COLOR_PRESSED, ColorToInt(LIGHTGRAY));

    AppContext context = {
        .mouseSensitivity = 0.5f,
        .selectedTexture = Assets::GetTexture("assets/textures/psa.png"),
        .selectedShape = Assets::GetShape("assets/models/shapes/cube.obj"),
        .texturesDir = "assets/textures",
        .shapesDir = "assets/models/shapes/"
    };

    //Editor modes
    PlaceMode placeMode(&context);
    PickMode texturePickMode(&context, PickMode::Mode::TEXTURES);
    PickMode shapePickMode(&context, PickMode::Mode::SHAPES);
    EditorMode *editorMode = &placeMode;

	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
        EditorMode *lastMode = editorMode;

        if (IsKeyPressed(KEY_TAB)) {
            if (editorMode == &placeMode) {
                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    editorMode = &shapePickMode;
                } else {
                    editorMode = &texturePickMode;
                }
            } else {
                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    editorMode = &shapePickMode;
                } else {
                    editorMode = &placeMode;
                }
            }
        }

        if (lastMode != editorMode) {
            lastMode->OnExit();
            editorMode->OnEnter();
        }

        //Update
        Assets::Update();
        editorMode->Update();

        //Draw
		BeginDrawing();
		
		ClearBackground(BLACK);

		editorMode->Draw();
		
        DrawRectangle(2, 2, 128, 32, DARKGRAY);
		DrawFPS(4, 4);

		EndDrawing();
	}
	
    Assets::Unload();

	CloseWindow();

	return 0;
}
