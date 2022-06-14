#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>

#include "app.hpp"
#include "assets.hpp"
#include "editor_mode.hpp"
#include "place_mode.hpp"
#include "pick_mode.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char **argv)
{
    //Window stuff
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Total Editor 3");
    SetWindowMinSize(640, 480);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
	InitAudioDevice();
    SetExitKey(KEY_NULL);

    Assets::Initialize();

    //RayGUI Styling
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(DARKGRAY));
    GuiSetFont(*Assets::GetFont());
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(LABEL, TEXT_COLOR_FOCUSED, ColorToInt(YELLOW));
    GuiSetStyle(LABEL, TEXT_COLOR_PRESSED, ColorToInt(LIGHTGRAY));
    GuiSetStyle(SCROLLBAR, SCROLL_SPEED, 64);
    GuiSetStyle(LISTVIEW, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));

    //Context
    AppContext context = {
        .undoStackSize = 30UL,
        .mouseSensitivity = 0.5f,
        .selectedTexture = Assets::GetTexture("assets/textures/psa.png"),
        .selectedShape = Assets::GetShape("assets/models/shapes/cube.obj"),
        .texturesDir = "assets/textures",
        .shapesDir = "assets/models/shapes/"
    };

    //Editor modes initialization
    std::unique_ptr<PlaceMode> placeMode      = std::make_unique<PlaceMode>(&context);
    std::unique_ptr<PickMode> texturePickMode = std::make_unique<PickMode>(&context, PickMode::Mode::TEXTURES);
    std::unique_ptr<PickMode> shapePickMode   = std::make_unique<PickMode>(&context, PickMode::Mode::SHAPES);
    EditorMode *editorMode = placeMode.get();

    //Main loop
	SetTargetFPS(300);
	while (!WindowShouldClose())
	{
        EditorMode *lastMode = editorMode;

        //Shortcuts for mode switching
        if (IsKeyPressed(KEY_TAB)) {
            if (editorMode == placeMode.get()) {
                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    editorMode = shapePickMode.get();
                } else {
                    editorMode = texturePickMode.get();
                }
            } else {
                if (IsKeyDown(KEY_LEFT_SHIFT)) {
                    if (editorMode == texturePickMode.get()) editorMode = shapePickMode.get();
                    else if (editorMode == shapePickMode.get()) editorMode = texturePickMode.get();
                } else {
                    editorMode = placeMode.get();
                }
            }
        }

        //Mode change handlers
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
		
		DrawFPS(4, GetScreenHeight() - 24);

		EndDrawing();
	}
    
    //Deinitialization
    Assets::Unload();
	CloseWindow();

	return 0;
}
