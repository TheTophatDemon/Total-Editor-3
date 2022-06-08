#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include <stdlib.h>

#include "tile.h"
#include "math_stuff.h"
#include "grid_extras.h"
#include "array_list.h"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const char* SHAPE_MODEL_PATH = "assets/models/shapes/";
const char* TEXTURE_PATH = "assets/textures/";

const Vector3 VECTOR3_UP = (Vector3) { 0.0f, 1.0f, 0.0f };

extern void MoveCamera(Camera*, float);

ModelList gShapes = { 0 };
size_t gShapesMeshCount = 0;

MaterialList gMaterialsInstanced = { 0 };
MaterialList gMaterialsNormal = { 0 };

typedef struct Cursor {
    Model model;
    size_t shapeIndex;
    size_t materialIndex;
    Angle angle;
    Vector3 position;
    float outlineScale;
} Cursor;

int main(int argc, char **argv)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Total Editor 3");
	InitAudioDevice();
	
	Font font = LoadFont("assets/fonts/dejavu.fnt");

    Shader mapShader = LoadShader("assets/shaders/map_geom.vs", "assets/shaders/map_geom.fs");
    mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(mapShader, "mvp");
    mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(mapShader, "viewPos");
    mapShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(mapShader, "instanceTransform");

    //Setup camera
	Camera camera = { 0 };
	camera.up = VECTOR3_UP;
	camera.fovy = 70.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	SetCameraMode(camera, CAMERA_PERSPECTIVE);

    float mouseSensitivity = 0.5f;

    //Get shapes

    int shapeFileCount = 0;
    char** shapeFiles = GetDirectoryFiles(SHAPE_MODEL_PATH, &shapeFileCount);

    LIST_RESERVE(Model, gShapes, shapeFileCount);

    gShapesMeshCount = 0;
    for (int f = 0; f < shapeFileCount; ++f) {
        char shapePath[strlen(SHAPE_MODEL_PATH) + strlen(shapeFiles[f]) + 1];
        strcpy(shapePath, SHAPE_MODEL_PATH);
        strcat(shapePath, shapeFiles[f]);
        if (IsFileExtension(shapePath, ".obj")) {
            Model model = LoadModel(shapePath);
            gShapesMeshCount += model.meshCount;
            LIST_APPEND(Model, gShapes, model);
        }
    }

    //Get textures and generate materials
    int textureFileCount = 0;
    char** textureFiles = GetDirectoryFiles(TEXTURE_PATH, &textureFileCount);

    LIST_RESERVE(Material, gMaterialsInstanced, textureFileCount);
    LIST_RESERVE(Material, gMaterialsNormal, textureFileCount);

    for (int f = 0; f < textureFileCount; ++f) {
        char texturePath[strlen(TEXTURE_PATH) + strlen(textureFiles[f]) + 1];
        strcpy(texturePath, TEXTURE_PATH);
        strcat(texturePath, textureFiles[f]);
        if (IsFileExtension(texturePath, ".png")) {
            Texture tex = LoadTexture(texturePath);
            //A copy of each material is made that has the `mapShader` assigned to it for rendering tiles.
            Material instancedMat = LoadMaterialDefault();
            SetMaterialTexture(&instancedMat, MATERIAL_MAP_ALBEDO, tex);
            instancedMat.shader = mapShader;
            LIST_APPEND(Material, gMaterialsInstanced, instancedMat);

            Material normalMat = LoadMaterialDefault();
            SetMaterialTexture(&normalMat, MATERIAL_MAP_ALBEDO, tex);
            normalMat.maps[MATERIAL_MAP_ALBEDO].color = WHITE;
            LIST_APPEND(Material, gMaterialsNormal, normalMat);
        }
    }

    ClearDirectoryFiles();

    //Generate cursor
    Cursor cursor = { 0 };
    cursor.model = gShapes.data[0];
    cursor.shapeIndex = 0;
    cursor.materialIndex = 0;
    cursor.position = Vector3Zero();
    cursor.angle = ANGLE_0;
    cursor.outlineScale = 1.0f;

    TileGrid tileGrid = NewTileGrid(100, 5, 100, 2.0f);
    Vector3 planeGridPos = (Vector3){(float)tileGrid.width / 2, 0, (float)tileGrid.length / 2};

    float globalTime = 0.0f;

	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
        globalTime += GetFrameTime();

		UpdateCamera(&camera);
        MoveCamera(&camera, mouseSensitivity);
        
        //Move editing plane
        if (GetMouseWheelMove() > EPSILON || GetMouseWheelMove() < -EPSILON) {
            planeGridPos.y = Clamp(planeGridPos.y + Sign(GetMouseWheelMove()), 0.0f, tileGrid.height - 1);
        }
        Vector3 planeWorldPos = GridToWorldPos(&tileGrid, planeGridPos, false);

        //Rotate cursor
        if (IsKeyPressed(KEY_Q)) {
            cursor.angle = AngleBack(cursor.angle);
        } else if (IsKeyPressed(KEY_E)) {
            cursor.angle = AngleForward(cursor.angle);
        }

        //Position cursor
        Ray pickRay = GetMouseRay(GetMousePosition(), camera);
        Vector3 gridMin = GetMinCorner(&tileGrid);
        Vector3 gridMax = GetMaxCorner(&tileGrid);
        RayCollision col = GetRayCollisionQuad(pickRay, 
            (Vector3){ gridMin.x, planeWorldPos.y, gridMin.z }, 
            (Vector3){ gridMax.x, planeWorldPos.y, gridMin.z }, 
            (Vector3){ gridMax.x, planeWorldPos.y, gridMax.z }, 
            (Vector3){ gridMin.x, planeWorldPos.y, gridMax.z });
        if (col.hit) {
            cursor.position = SnapToCelCenter(&tileGrid, col.point);
            cursor.position.y = planeWorldPos.y + tileGrid.spacing / 2.0f;
        }
        cursor.outlineScale = 1.125f + sinf(globalTime) * 0.125f;
		
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            //Place tiles
            Vector3 gridPos = WorldToGridPos(&tileGrid, cursor.position);
            SetTile(&tileGrid, gridPos.x, gridPos.y, gridPos.z, 
                (Tile){
                    &gShapes.data[cursor.shapeIndex],
                    cursor.angle,
                    &gMaterialsInstanced.data[cursor.materialIndex]
                }
            );
        } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            //Remove tiles
            Vector3 gridPos = WorldToGridPos(&tileGrid, cursor.position);
            UnsetTile(&tileGrid, gridPos.x, gridPos.y, gridPos.z);
        }

		BeginDrawing();
		
		ClearBackground(BLACK);

		BeginMode3D(camera);
		{
            DrawGridEx(
                planeWorldPos, 
                tileGrid.width+1, tileGrid.length+1, 
                tileGrid.spacing);

            DrawTileGrid(&tileGrid, mapShader);

            //Draw cursor
            Matrix cursorTransform = MatrixMultiply(
                MatrixRotateY(AngleRadians(cursor.angle)), 
                MatrixTranslate(cursor.position.x, cursor.position.y, cursor.position.z));
            for (size_t m = 0; m < gShapes.data[cursor.shapeIndex].meshCount; ++m) {
                DrawMesh(gShapes.data[cursor.shapeIndex].meshes[m], gMaterialsNormal.data[cursor.materialIndex], cursorTransform);
            }
            // DrawModelEx(cursor.model, cursor.position, VECTOR3_UP, AngleDegrees(cursor.angle), Vector3One(), WHITE);
            DrawCubeWires(
                cursor.position, 
                tileGrid.spacing * cursor.outlineScale, 
                tileGrid.spacing * cursor.outlineScale, 
                tileGrid.spacing * cursor.outlineScale, 
                MAGENTA);
		}
		EndMode3D();
		
		DrawFPS(4, 4);

		EndDrawing();
	}

    FreeTileGrid(tileGrid);
	
	CloseWindow();

	return 0;
}
