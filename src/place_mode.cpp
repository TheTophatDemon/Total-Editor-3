#include "place_mode.hpp"

#include "raymath.h"
#include "rlgl.h"

#include "grid_extras.h"
#include "math_stuff.hpp"
#include "assets.hpp"

const float CAMERA_HEIGHT_OFFSET = 10.0f;
const float CAMERA_MOVE_SPEED_MIN = 8.0f;
const float CAMERA_MOVE_SPEED_MAX = 64.0f;
const float CAMERA_ACCELERATION = 16.0f;

PlaceMode::PlaceMode(AppContext *context) 
    : _context(context), 
    _tileGrid(100, 5, 100, 2.0f)
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
    _cursor.shape = context->selectedShape;
    _cursor.texture = context->selectedTexture;
    _cursor.position = Vector3Zero();
    _cursor.angle = ANGLE_0;
    _cursor.outlineScale = 1.0f;

    //Editor grid and plane
    _planeGridPos = (Vector3){(float)_tileGrid.GetWidth() / 2, 0, (float)_tileGrid.GetLength() / 2};
    _planeWorldPos = _tileGrid.GridToWorldPos(_planeGridPos, false);
}

void PlaceMode::OnEnter() {

}

void PlaceMode::OnExit() {
    
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
        _cameraYaw += GetMouseDelta().x * _context->mouseSensitivity * GetFrameTime();
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

    _cursor.shape = _context->selectedShape;
    _cursor.texture = _context->selectedTexture;

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
    
    Vector3 cursorGridPos = _tileGrid.WorldToGridPos(_cursor.position);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && _cursor.shape && _cursor.texture) {
        //Place tiles
        _tileGrid.SetTile(cursorGridPos.x, cursorGridPos.y, cursorGridPos.z, 
            (Tile){
                _cursor.shape,
                _cursor.angle,
                Assets::GetMaterialForTexture(_cursor.texture, true)
            }
        );
    } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        //Remove tiles
        _tileGrid.UnsetTile(cursorGridPos.x, cursorGridPos.y, cursorGridPos.z);
    } else if (IsKeyDown(KEY_G)) {
        //(G)rab the shape from the tile under the cursor
        Model *shape  = _tileGrid.GetTile(cursorGridPos.x, cursorGridPos.y, cursorGridPos.z).shape;
        if (shape) _context->selectedShape = shape;
    } else if (IsKeyDown(KEY_T)) {
        //Pick the (T)exture from the tile under the cursor.
        Material *material = _tileGrid.GetTile(cursorGridPos.x, cursorGridPos.y, cursorGridPos.z).material;
        if (material) _context->selectedTexture = Assets::GetTextureForMaterial(material);
    }
}

void PlaceMode::Draw() {
    BeginMode3D(_camera);
    {
        DrawGridEx(
            Vector3Add(_planeWorldPos, (Vector3){ 0.0f, 0.1f, 0.0f }), //Adding the offset to prevent Z-fighting 
            _tileGrid.GetWidth()+1, _tileGrid.GetLength()+1, 
            _tileGrid.GetSpacing());

        _tileGrid.Draw();

        //Draw cursor
        if (_cursor.shape && _cursor.texture) {
            Matrix cursorTransform = MatrixMultiply(
                MatrixRotateY(AngleRadians(_cursor.angle)), 
                MatrixTranslate(_cursor.position.x, _cursor.position.y, _cursor.position.z));
            for (size_t m = 0; m < _cursor.shape->meshCount; ++m) {
                DrawMesh(_cursor.shape->meshes[m], *Assets::GetMaterialForTexture(_cursor.texture, false), cursorTransform);
            }
        }
        rlDisableDepthTest();
        DrawCubeWires(
            _cursor.position, 
            _tileGrid.GetSpacing() * _cursor.outlineScale, 
            _tileGrid.GetSpacing() * _cursor.outlineScale, 
            _tileGrid.GetSpacing() * _cursor.outlineScale, 
            MAGENTA);
        rlEnableDepthTest();
    }
    EndMode3D();
}