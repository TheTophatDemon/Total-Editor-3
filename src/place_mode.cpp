#include "place_mode.hpp"

#include "raymath.h"
#include "grid_extras.h"
#include "math_stuff.hpp"

extern std::vector<Model> gShapes;
extern size_t gShapesMeshCount;
extern std::vector<Material> gMaterialsNormal;

const float CAMERA_HEIGHT_OFFSET = 10.0f;
const float CAMERA_MOVE_SPEED_MIN = 8.0f;
const float CAMERA_MOVE_SPEED_MAX = 64.0f;
const float CAMERA_ACCELERATION = 16.0f;
const float CAMERA_MOUSE_SENSITIVITY = 0.5f;

PlaceMode::PlaceMode() 
    :_tileGrid(100, 5, 100, 2.0f)
{
    //Setup camera
	_camera = { 0 };
	_camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	_camera.fovy = 70.0f;
	_camera.projection = CAMERA_PERSPECTIVE;
	SetCameraMode(_camera, CAMERA_PERSPECTIVE);

    _cameraYaw = 0.0f;
    _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

    //Generate cursor
    _cursor = { 0 };
    _cursor.shapeIndex = 0;
    _cursor.materialIndex = 0;
    _cursor.position = Vector3Zero();
    _cursor.angle = ANGLE_0;
    _cursor.outlineScale = 1.0f;

    //Editor grid and plane
    _planeGridPos = (Vector3){(float)_tileGrid.GetWidth() / 2, 0, (float)_tileGrid.GetLength() / 2};
    _planeWorldPos = _tileGrid.GridToWorldPos(_planeGridPos, false);

    //Initialize instanced shader for map geometry
    _mapShader = LoadShader("assets/shaders/map_geom.vs", "assets/shaders/map_geom.fs");
    _mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
    _mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");
    _mapShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_mapShader, "instanceTransform");

    //Generate materials using the instanced shader.
    _instancedMaterials.reserve(gMaterialsNormal.size());
    for (const auto& material : gMaterialsNormal) {
        Material instancedMat = LoadMaterialDefault();
        SetMaterialTexture(&instancedMat, MATERIAL_MAP_ALBEDO, material.maps[MATERIAL_MAP_ALBEDO].texture);
        instancedMat.shader = _mapShader;
        _instancedMaterials.push_back(instancedMat);
    }
}


void PlaceMode::MoveCamera() {
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
        _cameraMoveSpeed += CAMERA_ACCELERATION * GetFrameTime();
    } else {
        _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;
    }
    _cameraMoveSpeed = Clamp(_cameraMoveSpeed, CAMERA_MOVE_SPEED_MIN, CAMERA_MOVE_SPEED_MAX);
    
    cameraMovement = Vector3Scale(Vector3Normalize(cameraMovement), _cameraMoveSpeed * GetFrameTime());
    Vector3 rotatedMovement = Vector3Transform(cameraMovement, MatrixRotateY(_cameraYaw - PI / 2.0f));
    _camera.target = Vector3Add(_camera.target, rotatedMovement);

    if (IsKeyPressed(KEY_R)) {
        _camera.target = Vector3Zero();
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
        _cameraYaw += GetMouseDelta().x * CAMERA_MOUSE_SENSITIVITY * GetFrameTime();
    }

    _camera.position = (Vector3){ 
        _camera.target.x + cosf(_cameraYaw) * CAMERA_HEIGHT_OFFSET, 
        _camera.target.y + CAMERA_HEIGHT_OFFSET, 
        _camera.target.z + sinf(_cameraYaw) * CAMERA_HEIGHT_OFFSET };
}

void PlaceMode::Update() {
    UpdateCamera(&_camera);
    MoveCamera();
    
    //Move editing plane
    if (GetMouseWheelMove() > EPSILON || GetMouseWheelMove() < -EPSILON) {
        _planeGridPos.y = Clamp(_planeGridPos.y + Sign(GetMouseWheelMove()), 0.0f, _tileGrid.GetHeight() - 1);
    }
    _planeWorldPos = _tileGrid.GridToWorldPos(_planeGridPos, false);

    //Rotate cursor
    if (IsKeyPressed(KEY_Q)) {
        _cursor.angle = AngleBack(_cursor.angle);
    } else if (IsKeyPressed(KEY_E)) {
        _cursor.angle = AngleForward(_cursor.angle);
    }

    //Position cursor
    Ray pickRay = GetMouseRay(GetMousePosition(), _camera);
    Vector3 gridMin = _tileGrid.GetMinCorner();
    Vector3 gridMax = _tileGrid.GetMaxCorner();
    RayCollision col = GetRayCollisionQuad(pickRay, 
        (Vector3){ gridMin.x, _planeWorldPos.y, gridMin.z }, 
        (Vector3){ gridMax.x, _planeWorldPos.y, gridMin.z }, 
        (Vector3){ gridMax.x, _planeWorldPos.y, gridMax.z }, 
        (Vector3){ gridMin.x, _planeWorldPos.y, gridMax.z });
    if (col.hit) {
        _cursor.position = _tileGrid.SnapToCelCenter(col.point);
        _cursor.position.y = _planeWorldPos.y + _tileGrid.GetSpacing() / 2.0f;
    }
    _cursor.outlineScale = 1.125f + sinf(GetTime()) * 0.125f;
    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        //Place tiles
        Vector3 gridPos = _tileGrid.WorldToGridPos(_cursor.position);
        _tileGrid.SetTile(gridPos.x, gridPos.y, gridPos.z, 
            (Tile){
                &gShapes[_cursor.shapeIndex],
                _cursor.angle,
                &_instancedMaterials[_cursor.materialIndex]
            }
        );
    } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        //Remove tiles
        Vector3 gridPos = _tileGrid.WorldToGridPos(_cursor.position);
        _tileGrid.UnsetTile(gridPos.x, gridPos.y, gridPos.z);
    }
}

void PlaceMode::Draw() {
    BeginMode3D(_camera);
    {
        DrawGridEx(
            _planeWorldPos, 
            _tileGrid.GetWidth()+1, _tileGrid.GetLength()+1, 
            _tileGrid.GetSpacing());

        _tileGrid.Draw(_mapShader);

        //Draw cursor
        Matrix cursorTransform = MatrixMultiply(
            MatrixRotateY(AngleRadians(_cursor.angle)), 
            MatrixTranslate(_cursor.position.x, _cursor.position.y, _cursor.position.z));
        for (size_t m = 0; m < gShapes[_cursor.shapeIndex].meshCount; ++m) {
            DrawMesh(gShapes[_cursor.shapeIndex].meshes[m], gMaterialsNormal[_cursor.materialIndex], cursorTransform);
        }

        DrawCubeWires(
            _cursor.position, 
            _tileGrid.GetSpacing() * _cursor.outlineScale, 
            _tileGrid.GetSpacing() * _cursor.outlineScale, 
            _tileGrid.GetSpacing() * _cursor.outlineScale, 
            MAGENTA);
    }
    EndMode3D();
}