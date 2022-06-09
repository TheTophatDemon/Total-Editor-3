#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include <stdlib.h>
#include <vector>
#include <string>

#include "editor_mode.hpp"
#include "place_mode.hpp"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const char* SHAPE_MODEL_PATH = "assets/models/shapes/";
const char* TEXTURE_PATH = "assets/textures/";

std::vector<Model> gShapes;
size_t gShapesMeshCount = 0;
std::vector<Material> gMaterialsNormal;

int main(int argc, char **argv)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Total Editor 3");
	InitAudioDevice();
	
	Font font = LoadFont("assets/fonts/dejavu.fnt");

    //Get shapes
    int shapeFileCount = 0;
    char** shapeFiles = GetDirectoryFiles(SHAPE_MODEL_PATH, &shapeFileCount);

    gShapes.reserve(shapeFileCount);

    gShapesMeshCount = 0;
    for (int f = 0; f < shapeFileCount; ++f) {
        std::string shapePath = std::string(SHAPE_MODEL_PATH) + shapeFiles[f];
        if (IsFileExtension(shapePath.c_str(), ".obj")) {
            Model model = LoadModel(shapePath.c_str());
            gShapesMeshCount += model.meshCount;
            gShapes.push_back(model);
        }
    }

    //Get textures and generate materials
    int textureFileCount = 0;
    char **textureFiles = GetDirectoryFiles(TEXTURE_PATH, &textureFileCount);

    gMaterialsNormal.reserve(textureFileCount);

    size_t textureCount = 0;

    for (int f = 0; f < textureFileCount; ++f) {
        std::string texturePath = std::string(TEXTURE_PATH) + textureFiles[f];
        if (IsFileExtension(texturePath.c_str(), ".png")) {
            Texture tex = LoadTexture(texturePath.c_str());
            ++textureCount;

            Material normalMat = LoadMaterialDefault();
            SetMaterialTexture(&normalMat, MATERIAL_MAP_ALBEDO, tex);
            normalMat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;
            gMaterialsNormal.push_back(normalMat);
        }
    }

    ClearDirectoryFiles();

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
