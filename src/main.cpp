#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include <stdlib.h>
#include <vector>
#include <string>

#include "assets.hpp"
#include "editor_mode.hpp"
#include "place_mode.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

int main(int argc, char **argv)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Total Editor 3");
	InitAudioDevice();

    Assets::Initialize();

    //Editor modes
    PlaceMode placeMode;
    EditorMode *editorMode = &placeMode;

	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
        if (IsKeyPressed(KEY_TAB)) {
            //Switch modes, later..
        }

        //Update
        editorMode->Update();

        //Draw
		BeginDrawing();
		
		ClearBackground(BLACK);

		editorMode->Draw();
		
		DrawFPS(4, 4);

		EndDrawing();
	}
	
	CloseWindow();

	return 0;
}
