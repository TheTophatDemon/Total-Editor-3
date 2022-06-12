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
	_camera.up = VEC3_UP;
	_camera.fovy = 70.0f;
	_camera.projection = CAMERA_PERSPECTIVE;
	SetCameraMode(_camera, CAMERA_PERSPECTIVE);

    _cameraYaw = 0.0f;
    _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

    //Generate cursor
    _cursor = { 0 };
    _cursor.tile = (Tile) {
        .shape = context->selectedShape,
        .angle = ANGLE_0,
        .texture = context->selectedTexture
    };
    _cursor.position = Vector3Zero();
    _cursor.outlineScale = 1.0f;

    //Editor grid and plane
    _planeGridPos = (Vector3){(float)_tileGrid.GetWidth() / 2, 0, (float)_tileGrid.GetLength() / 2};
    _planeWorldPos = _tileGrid.GridToWorldPos(_planeGridPos, false);
}

void PlaceMode::OnEnter() {
    _cursor.tile.shape = _context->selectedShape;
    _cursor.tile.texture = _context->selectedTexture;
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
    } else if (IsKeyDown(KEY_C)) {
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

    //Update the cursor
    
    //Rotate cursor
    if (IsKeyPressed(KEY_Q)) {
        _cursor.tile.angle = AngleBack(_cursor.tile.angle);
    } else if (IsKeyPressed(KEY_E)) {
        _cursor.tile.angle = AngleForward(_cursor.tile.angle);
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
    
    //Perform Tile operations
    Vector3 cursorGridPos = _tileGrid.WorldToGridPos(_cursor.position);
    size_t i = (size_t)cursorGridPos.x; 
    size_t j = (size_t)cursorGridPos.y;
    size_t k = (size_t)cursorGridPos.z;
    Tile underTile = _tileGrid.GetTile(i, j, k);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && _cursor.tile.shape && _cursor.tile.texture) {
        //Place tiles
        if (underTile != _cursor.tile)
        {
            DoAction(
                QueueTileAction(i, j, k, 1, 1, 1, _cursor.tile)
            );
        }
    } else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        //Remove tiles
        if (underTile.shape != nullptr || underTile.texture != nullptr)
        {
            DoAction(
                QueueTileAction(
                    i, j, k, 1, 1, 1,
                    (Tile) {nullptr, ANGLE_0, nullptr}
                )
            );
        }
    } else if (IsKeyDown(KEY_G)) {
        //(G)rab the shape from the tile under the cursor
        Model *shape  = _tileGrid.GetTile(cursorGridPos.x, cursorGridPos.y, cursorGridPos.z).shape;
        if (shape) 
        {
            _cursor.tile.shape = shape;
            _context->selectedShape = shape;
        }
    } else if (IsKeyDown(KEY_T)) {
        //Pick the (T)exture from the tile under the cursor.
        Texture2D *tex = _tileGrid.GetTile(cursorGridPos.x, cursorGridPos.y, cursorGridPos.z).texture;
        if (tex) 
        {
            _cursor.tile.texture = tex;
            _context->selectedTexture = tex;
        }
    }

    //Undo and redo
    if (IsKeyDown(KEY_LEFT_CONTROL))
    {
        if (IsKeyPressed(KEY_Z) && !_undoHistory.empty())
        {
            //Undo (Ctrl + Z)
            TileAction &action = _undoHistory.back();
            UndoAction(action);
            _redoHistory.push_back(action);
            _undoHistory.pop_back();
        }
        else if (IsKeyPressed(KEY_Y) && !_redoHistory.empty())
        {
            //Redo (Ctrl + Y)
            TileAction &action = _redoHistory.back();
            DoAction(action);
            _undoHistory.push_back(action);
            _redoHistory.pop_back();
        }
    }
}

void PlaceMode::Draw() {
    BeginMode3D(_camera);
    {
        DrawGridEx(
            Vector3Add(_planeWorldPos, (Vector3){ 0.0f, 0.05f, 0.0f }), //Adding the offset to prevent Z-fighting 
            _tileGrid.GetWidth()+1, _tileGrid.GetLength()+1, 
            _tileGrid.GetSpacing());

        _tileGrid.Draw();

        //Draw cursor
        if (_cursor.tile.shape && _cursor.tile.texture) {
            Matrix cursorTransform = MatrixMultiply(
                MatrixRotateY(AngleRadians(_cursor.tile.angle)), 
                MatrixTranslate(_cursor.position.x, _cursor.position.y, _cursor.position.z));
            for (size_t m = 0; m < _cursor.tile.shape->meshCount; ++m) {
                DrawMesh(_cursor.tile.shape->meshes[m], *Assets::GetMaterialForTexture(_cursor.tile.texture, false), cursorTransform);
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

PlaceMode::TileAction &PlaceMode::QueueTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile)
{
    _undoHistory.push_back((TileAction){
        .i = i, 
        .j = j, 
        .k = k, 
        .prevState = _tileGrid.Subsection(i, j, k, w, h, l),
        .newState = TileGrid(w, h, l, _tileGrid.GetSpacing(), newTile) 
    });
    if (_undoHistory.size() > _context->undoStackSize) _undoHistory.pop_front();
    _redoHistory.clear();

    return _undoHistory.back();
}

void PlaceMode::DoAction(TileAction &action)
{
    _tileGrid.CopyTiles(action.i, action.j, action.k, action.newState);
}

void PlaceMode::UndoAction(TileAction &action)
{
    _tileGrid.CopyTiles(action.i, action.j, action.k, action.prevState);
}