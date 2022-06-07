#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include <stdlib.h>
#include <vector>
#include <iostream>
#include <string>

#include "tile.h"
#include "math_stuff.h"

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const float CAMERA_HEIGHT_OFFSET = 10.0f;
const float CAMERA_MOVE_SPEED_MIN = 8.0f;
const float CAMERA_MOVE_SPEED_MAX = 32.0f;
const float CAMERA_ACCELERATION = 4.0f;

const std::string SHAPE_MODEL_PATH = "assets/models/shapes/";

const Vector3 VECTOR3_UP = (Vector3) { 0.0f, 1.0f, 0.0f };

typedef struct Cursor {
    Model model;
    size_t shapeIndex;
    Angle angle;
    Vector3 position;
    float outlineScale;
} Cursor;

//Draw a grid centered at the given Vector3 `position`, with a rectangular size given by `slicesX` and `slicesZ`.
void DrawGridEx(Vector3 position, int slicesX, int slicesZ, float spacing)
{
    int halfSlicesX = slicesX/2;
    int halfSlicesZ = slicesZ/2;

    rlCheckRenderBatchLimit((slicesX + slicesZ + 4)*2);

    rlBegin(RL_LINES);
        for (int i = -halfSlicesX; i <= halfSlicesX; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f(position.x + (float)i*spacing, position.y, position.z + (float)-halfSlicesZ*spacing);
            rlVertex3f(position.x + (float)i*spacing, position.y, position.z + (float)halfSlicesZ*spacing);
        }

        for (int j = -halfSlicesZ; j <= halfSlicesZ; j++) 
        {
            if (j == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f(position.x + (float)-halfSlicesX*spacing, position.y, position.z + (float)j*spacing);
            rlVertex3f(position.x + (float)halfSlicesX*spacing, position.y, position.z + (float)j*spacing);
        }
    rlEnd();
}

int main(int argc, char **argv)
{
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Total Editor 3");
	InitAudioDevice();
	
	Font font = LoadFont("assets/fonts/dejavu.fnt");

    Texture2D texture = LoadTexture("assets/original/textures/spacewall.png");

    Shader mapShader = LoadShader("assets/shaders/map_geom.vs", "assets/shaders/map_geom.fs");
    mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(mapShader, "mvp");
    mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(mapShader, "viewPos");
    mapShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(mapShader, "instanceTransform");

    //Setup camera
	Camera camera = { 0 };
	camera.position = (Vector3){ 0.0f, CAMERA_HEIGHT_OFFSET, CAMERA_HEIGHT_OFFSET };
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 70.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	SetCameraMode(camera, CAMERA_PERSPECTIVE);

    float cameraYaw = 0.0f;
    float cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;
    float mouseSensitivity = 0.5f;

    //Get shapes
    int shapeFileCount = 0;
    char** shapeFiles = GetDirectoryFiles(SHAPE_MODEL_PATH.c_str(), &shapeFileCount);
    std::vector<Model> shapes;
    for (int f = 0; f < shapeFileCount; ++f) {
        std::string shapePath = SHAPE_MODEL_PATH + shapeFiles[f];

        if (IsFileExtension(shapePath.c_str(), ".obj")) {
            Model model = LoadModel(shapePath.c_str());
            //Assign the instanced shader
            for (int m = 0; m < model.materialCount; ++m) {
                model.materials[m].shader = mapShader;
            }
            shapes.push_back(model);
        }

        RL_FREE(shapeFiles[f]);
    }
    RL_FREE(shapeFiles);

    //Generate cursor
    Cursor cursor = { 0 };
    cursor.model = shapes[0];
    cursor.model.materials = (Material*)RL_CALLOC(cursor.model.materialCount, sizeof(Material));
    cursor.shapeIndex = 0;
    for (int m = 0; m < cursor.model.materialCount; ++m) {
        cursor.model.materials[m] = LoadMaterialDefault();
        SetMaterialTexture(&cursor.model.materials[m], MATERIAL_MAP_DIFFUSE, texture);
    }
    cursor.position = Vector3Zero();
    cursor.angle = ANGLE_0;
    cursor.outlineScale = 1.0f;

    TileGrid tileGrid = TileGrid(100, 5, 100, 2.0f);
    Vector3 planeGridPos = Vector3{(float)tileGrid.GetWidth() / 2, 0, (float)tileGrid.GetLength() / 2};

    float globalTime = 0.0f;

	SetTargetFPS(60);
	while (!WindowShouldClose())
	{
        globalTime += GetFrameTime();

		UpdateCamera(&camera);

        //Move editing plane
        if (GetMouseWheelMove() > EPSILON || GetMouseWheelMove() < -EPSILON) {
            planeGridPos.y = Clamp(planeGridPos.y + Sign(GetMouseWheelMove()), 0.0f, tileGrid.GetHeight() - 1);
        }
        Vector3 planeWorldPos = tileGrid.GridToWorldPos(planeGridPos, false);

        //Camera controls
        Vector3 cameraMovement = Vector3Zero();
        if (IsKeyDown(KEY_D)) {
            cameraMovement.x = 1.0f;
        } else if (IsKeyDown(KEY_A)) {
            cameraMovement.x = -1.0f;
        }

        if (IsKeyDown(KEY_W)) {
            cameraMovement.z = -1.0f;
        } else if (IsKeyDown(KEY_S)) {
            cameraMovement.z = 1.0f;
        }

        if (IsKeyDown(KEY_SPACE)) {
            cameraMovement.y = 1.0f;
        } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
            cameraMovement.y = -1.0f;
        }

        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_A) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S)) {
            cameraMoveSpeed += CAMERA_ACCELERATION * GetFrameTime();
        } else {
            cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;
        }
        cameraMoveSpeed = Clamp(cameraMoveSpeed, CAMERA_MOVE_SPEED_MIN, CAMERA_MOVE_SPEED_MAX);
        
        cameraMovement = Vector3Scale(Vector3Normalize(cameraMovement), cameraMoveSpeed * GetFrameTime());
        Vector3 rotatedMovement = Vector3Transform(cameraMovement, MatrixRotateY(cameraYaw - PI / 2.0f));
        camera.target = Vector3Add(camera.target, rotatedMovement);

        if (IsKeyPressed(KEY_R)) {
            camera.target = Vector3Zero();
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
            cameraYaw += GetMouseDelta().x * mouseSensitivity * GetFrameTime();
        }

        camera.position = Vector3{ 
            camera.target.x + cosf(cameraYaw) * CAMERA_HEIGHT_OFFSET, 
            camera.target.y + CAMERA_HEIGHT_OFFSET, 
            camera.target.z + sinf(cameraYaw) * CAMERA_HEIGHT_OFFSET };

        float cameraPos[3] = { camera.position.x, camera.position.y, camera.position.z };
        SetShaderValue(mapShader, mapShader.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

        if (IsKeyPressed(KEY_Q)) {
            cursor.angle = AngleBack(cursor.angle);
        } else if (IsKeyPressed(KEY_E)) {
            cursor.angle = AngleForward(cursor.angle);
        }

        //Position cursor
        Ray pickRay = GetMouseRay(GetMousePosition(), camera);
        Vector3 gridMin = tileGrid.GetMinCorner();
        Vector3 gridMax = tileGrid.GetMaxCorner();
        RayCollision col = GetRayCollisionQuad(pickRay, 
            (Vector3){ gridMin.x, planeWorldPos.y, gridMin.z }, 
            (Vector3){ gridMax.x, planeWorldPos.y, gridMin.z }, 
            (Vector3){ gridMax.x, planeWorldPos.y, gridMax.z }, 
            (Vector3){ gridMin.x, planeWorldPos.y, gridMax.z });
        if (col.hit) {
            cursor.position = tileGrid.SnapToCelCenter(col.point);
            cursor.position.y = planeWorldPos.y + tileGrid.GetSpacing() / 2.0f;
        }
        cursor.outlineScale = 1.125f + sinf(globalTime) * 0.125f;
		
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            //Place tiles
            Vector3 gridPos = tileGrid.WorldToGridPos(cursor.position);
            tileGrid.SetTile(gridPos.x, gridPos.y, gridPos.z, 
                Tile(
                    &shapes[cursor.shapeIndex],
                    cursor.angle,
                    &texture
                )
            );
        } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            //Remove tiles
            Vector3 gridPos = tileGrid.WorldToGridPos(cursor.position);
            tileGrid.UnsetTile(gridPos.x, gridPos.y, gridPos.z);
        }

		BeginDrawing();
		
		ClearBackground(BLACK);

		BeginMode3D(camera);
		{
            DrawGridEx(
                planeWorldPos, 
                tileGrid.GetWidth()+1, tileGrid.GetLength()+1, 
                tileGrid.GetSpacing());

            tileGrid.Draw(mapShader);

            //Draw cursor
            DrawModelEx(cursor.model, cursor.position, VECTOR3_UP, AngleDegrees(cursor.angle), Vector3One(), WHITE);
            DrawCubeWires(
                cursor.position, 
                tileGrid.GetSpacing() * cursor.outlineScale, 
                tileGrid.GetSpacing() * cursor.outlineScale, 
                tileGrid.GetSpacing() * cursor.outlineScale, 
                MAGENTA);
		}
		EndMode3D();
		
		DrawFPS(4, 4);

		EndDrawing();
	}

    for (const Model& m : shapes) {
        UnloadModel(m);
    }
    
	UnloadFont(font);
	CloseWindow();

	return 0;
}
