#include "place_mode.hpp"

#include "raymath.h"
#include "rlgl.h"
#include "extras/raygui.h"

#include "grid_extras.h"
#include "math_stuff.hpp"
#include "assets.hpp"

#define CAMERA_PITCH_LIMIT (PI / 2.5f)
#define CAMERA_MOVE_SPEED_MIN   8.0f
#define CAMERA_MOVE_SPEED_MAX   64.0f
#define CAMERA_ACCELERATION     16.0f

static Rectangle topBar = (Rectangle) { 0 };

PlaceMode::PlaceMode(AppContext *context) 
    : _context(context), 
    _tileGrid(100, 5, 100, 2.0f)
{
    _menuBar.AddMenu("FILE", {"NEW", "OPEN", "SAVE", "SAVE AS", "RESIZE"});
    _menuBar.AddMenu("VIEW", {"TEXTURES", "SHAPES", "THINGS"});
    _menuBar.AddMenu("CONFIG", {"ASSET PATHS", "SETTINGS"});
    _menuBar.AddMenu("INFO", {"ABOUT", "SHORTCUTS", "INSTRUCTIONS"});

    //Setup camera
	_camera = { 0 };
	_camera.up = VEC3_UP;
	_camera.fovy = 70.0f;
	_camera.projection = CAMERA_PERSPECTIVE;

    _cameraYaw = 0.0f;
    _cameraPitch = PI / 4.0f;
    _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

    //Setup cursor
    _cursor = { 0 };
    _cursor.tile = (Tile) {
        .shape = context->selectedShape,
        .angle = ANGLE_0,
        .texture = context->selectedTexture
    };
    _cursor.startPosition = _cursor.endPosition = Vector3Zero();
    _cursor.outlineScale = 1.0f;

    //Editor grid and plane
    _planeGridPos = (Vector3){(float)_tileGrid.GetWidth() / 2, 0, (float)_tileGrid.GetLength() / 2};
    _planeWorldPos = _tileGrid.GridToWorldPos(_planeGridPos, false);

    _cursor.outlineScale = 1.125f;
}

void PlaceMode::OnEnter() 
{
    _cursor.tile.shape = _context->selectedShape;
    _cursor.tile.texture = _context->selectedTexture;
}

void PlaceMode::OnExit() 
{
    _context->selectedShape = _cursor.tile.shape;
    _context->selectedTexture = _cursor.tile.texture;
}

void PlaceMode::MoveCamera() 
{
    //Camera controls
    Vector3 cameraMovement = Vector3Zero();
    if (IsKeyDown(KEY_D)) 
    {
        cameraMovement.x = 1.0f;
    } 
    else if (IsKeyDown(KEY_A)) 
    {
        cameraMovement.x = -1.0f;
    }

    if (IsKeyDown(KEY_W))
    {
        cameraMovement.z = -1.0f;
    }
    else if (IsKeyDown(KEY_S))
    {
        cameraMovement.z = 1.0f;
    }

    if (IsKeyDown(KEY_SPACE))
    {
        cameraMovement.y = 1.0f;
    }
    else if (IsKeyDown(KEY_C))
    {
        cameraMovement.y = -1.0f;
    }

    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_A) || IsKeyDown(KEY_W) || IsKeyDown(KEY_S))
    {
        _cameraMoveSpeed += CAMERA_ACCELERATION * GetFrameTime();
    }
    else
    {
        _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;
    }
    _cameraMoveSpeed = Clamp(_cameraMoveSpeed, CAMERA_MOVE_SPEED_MIN, CAMERA_MOVE_SPEED_MAX);
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) 
    {
        _cameraYaw += GetMouseDelta().x * _context->mouseSensitivity * GetFrameTime();
        _cameraPitch += GetMouseDelta().y * _context->mouseSensitivity * GetFrameTime();
        _cameraPitch = Clamp(_cameraPitch, -CAMERA_PITCH_LIMIT, CAMERA_PITCH_LIMIT);
    }

    cameraMovement = Vector3Scale(Vector3Normalize(cameraMovement), _cameraMoveSpeed * GetFrameTime());
    Matrix cameraRotation = MatrixMultiply(MatrixRotateX(_cameraPitch), MatrixRotateY(_cameraYaw));
    Vector3 rotatedMovement = Vector3Transform(cameraMovement, cameraRotation);
    
    _camera.position = Vector3Add(_camera.position, rotatedMovement);
    _camera.target = Vector3Add(_camera.position, Vector3Transform(VEC3_FORWARD, cameraRotation));
}

void PlaceMode::UpdateCursor()
{
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
        _cursor.endPosition = _tileGrid.SnapToCelCenter(col.point);
        _cursor.endPosition.y = _planeWorldPos.y + _tileGrid.GetSpacing() / 2.0f;
    }
    //_cursor.outlineScale = 1.125f + sinf(GetTime()) * 0.125f;
    
    //If shift is not held, the cursor only applies to the tile the cursor is on right now.
    bool multiSelect = false;
    if (!IsKeyDown(KEY_LEFT_SHIFT))
    {
        _cursor.startPosition = _cursor.endPosition;
    }
    else
    {
        multiSelect = true;
    }

    //Perform Tile operations
    Vector3 cursorStartGridPos = _tileGrid.WorldToGridPos(_cursor.startPosition);
    Vector3 cursorEndGridPos = _tileGrid.WorldToGridPos(_cursor.endPosition);
    Vector3 gridPosMin = Vector3Min(cursorStartGridPos, cursorEndGridPos);
    Vector3 gridPosMax = Vector3Max(cursorStartGridPos, cursorEndGridPos);
    size_t i = (size_t)gridPosMin.x; 
    size_t j = (size_t)gridPosMin.y;
    size_t k = (size_t)gridPosMin.z;
    size_t w = (size_t)gridPosMax.x - i + 1;
    size_t h = (size_t)gridPosMax.y - j + 1;
    size_t l = (size_t)gridPosMax.z - k + 1;
    Tile underTile = _tileGrid.GetTile(i, j, k);
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !multiSelect) {
        //Place tiles
        if (underTile != _cursor.tile)
        {
            DoAction(
                QueueTileAction(i, j, k, 1, 1, 1, _cursor.tile)
            );
        }
    }
    else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && multiSelect)
    {
        //Place tiles rectangle
        DoAction(
            QueueTileAction(i, j, k, w, h, l, _cursor.tile)
        );
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !multiSelect) 
    {
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
    } 
    else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && multiSelect)
    {
        //Remove tiles RECTANGLE
        DoAction(
            QueueTileAction(
                i, j, k, w, h, l,
                (Tile) {nullptr, ANGLE_0, nullptr}
            )
        );
    }
    else if (IsKeyDown(KEY_G)) 
    {
        //(G)rab the shape from the tile under the cursor
        Model *shape  = _tileGrid.GetTile(cursorEndGridPos.x, cursorEndGridPos.y, cursorEndGridPos.z).shape;
        if (shape) 
        {
            _cursor.tile.shape = shape;
        }
    } 
    else if (IsKeyDown(KEY_T)) 
    {
        //Pick the (T)exture from the tile under the cursor.
        Texture2D *tex = _tileGrid.GetTile(cursorEndGridPos.x, cursorEndGridPos.y, cursorEndGridPos.z).texture;
        if (tex) 
        {
            _cursor.tile.texture = tex;
        }
    }
}

void PlaceMode::Update() {
    _menuBar.Update();

    if (!_menuBar.IsFocused())
    {
        MoveCamera();
        
        //Move editing plane
        if (GetMouseWheelMove() > EPSILON || GetMouseWheelMove() < -EPSILON) {
            _planeGridPos.y = Clamp(_planeGridPos.y + Sign(GetMouseWheelMove()), 0.0f, _tileGrid.GetHeight() - 1);
        }
        _planeWorldPos = _tileGrid.GridToWorldPos(_planeGridPos, false);

        if (IsKeyPressed(KEY_H))
        {
            if (IsKeyDown(KEY_LEFT_ALT))
            {
                _tileGrid.ShowAllLayers();
            }
            else if (IsKeyDown(KEY_LEFT_SHIFT))
            {
                _tileGrid.SoloLayer((int) _planeGridPos.y);
            }
            else
            {
                _tileGrid.ToggleShowLayer((int) _planeGridPos.y);
            }
        }

        if (!CheckCollisionPointRec(GetMousePosition(), topBar))
        {
            UpdateCursor();
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
        if (!IsKeyDown(KEY_LEFT_SHIFT) && _cursor.tile.shape && _cursor.tile.texture) {
            Matrix cursorTransform = MatrixMultiply(
                MatrixRotateY(AngleRadians(_cursor.tile.angle)), 
                MatrixTranslate(_cursor.endPosition.x, _cursor.endPosition.y, _cursor.endPosition.z));
            for (size_t m = 0; m < _cursor.tile.shape->meshCount; ++m) {
                DrawMesh(_cursor.tile.shape->meshes[m], *Assets::GetMaterialForTexture(_cursor.tile.texture, false), cursorTransform);
            }
        }
        rlDisableDepthTest();
        DrawCubeWires(
            Vector3Scale(Vector3Add(_cursor.startPosition, _cursor.endPosition), 0.5f), 
            fabs(_cursor.endPosition.x - _cursor.startPosition.x) + _tileGrid.GetSpacing() * _cursor.outlineScale, 
            fabs(_cursor.endPosition.y - _cursor.startPosition.y) + _tileGrid.GetSpacing() * _cursor.outlineScale, 
            fabs(_cursor.endPosition.z - _cursor.startPosition.z) + _tileGrid.GetSpacing() * _cursor.outlineScale, 
            MAGENTA);
        rlEnableDepthTest();
    }
    EndMode3D();

    topBar = (Rectangle) { 0, 0, (float)GetScreenWidth(), 32 };
    DrawRectangleGradientV(topBar.x, topBar.y, topBar.width, topBar.height, GRAY, DARKGRAY);
    _menuBar.Draw((Rectangle) { topBar.x, topBar.y, topBar.width / 2.0f, topBar.height });
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