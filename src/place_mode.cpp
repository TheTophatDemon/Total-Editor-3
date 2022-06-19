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

PlaceMode::PlaceMode(MapMan &mapMan, PlaceMode::Mode mode) 
    : _mapMan(mapMan),
      _mode(mode),
      _layerViewMax(mapMan.Map().GetHeight() - 1),
      _layerViewMin(0)
{
    //Setup camera
	_camera = { 0 };
	_camera.up = VEC3_UP;
	_camera.fovy = 70.0f;
	_camera.projection = CAMERA_PERSPECTIVE;
    _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

    //Setup cursor
    _cursor.tile = (Tile) {
        .shape = Assets::GetShape("assets/models/shapes/cube.obj"),
        .angle = ANGLE_0,
        .texture = Assets::GetTexture("assets/textures/brickwall.png"),
        false
    };
    _cursor.brush = TileGrid(1, 1, 1, mapMan.Map().GetSpacing());
    _cursor.brushMode = false;
    _cursor.startPosition = _cursor.endPosition = Vector3Zero();
    _cursor.outlineScale = 1.125f;

    ResetCamera();
    ResetGrid();
}

void PlaceMode::OnEnter() 
{
    _cursor.brushMode = false;
}

void PlaceMode::OnExit() 
{
}

void PlaceMode::ResetCamera()
{
    _camera.position = _mapMan.Map().GetCenterPos();
    _camera.target = _camera.position;
    _cameraYaw = 0.0f;
    _cameraPitch = PI / 4.0f;
    _cursor.startPosition = _cursor.endPosition = Vector3Zero();
}

void PlaceMode::ResetGrid()
{
    //Editor grid and plane
    _planeGridPos = (Vector3){ (float)_mapMan.Map().GetWidth() / 2.0f, 0, (float)_mapMan.Map().GetLength() / 2.0f };
    _planeWorldPos = _mapMan.Map().GridToWorldPos(_planeGridPos, false);
    _layerViewMin = 0;
    _layerViewMax = _mapMan.Map().GetHeight() - 1;
    _cursor.startPosition = _cursor.endPosition = Vector3Zero();
    _cursor.brushMode = false;
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
        _cameraYaw += GetMouseDelta().x * App::Get()->GetMouseSensitivity() * GetFrameTime();
        _cameraPitch += GetMouseDelta().y * App::Get()->GetMouseSensitivity() * GetFrameTime();
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
    if (IsKeyPressed(KEY_Q))
    {
        _cursor.tile.angle = AngleBack(_cursor.tile.angle);
    }
    else if (IsKeyPressed(KEY_E))
    {
        _cursor.tile.angle = AngleForward(_cursor.tile.angle);
    }
    //Flip
    if (IsKeyPressed(KEY_F))
    {
        _cursor.tile.flipped = !_cursor.tile.flipped;
    }
    //Reset tile orientation
    if (IsKeyPressed(KEY_R))
    {
        _cursor.tile.angle = ANGLE_0;
        _cursor.tile.flipped = false;
    }

    //Position cursor
    Ray pickRay = GetMouseRay(GetMousePosition(), _camera);
    Vector3 gridMin = _mapMan.Map().GetMinCorner();
    Vector3 gridMax = _mapMan.Map().GetMaxCorner();
    RayCollision col = GetRayCollisionQuad(pickRay, 
        (Vector3){ gridMin.x, _planeWorldPos.y, gridMin.z }, 
        (Vector3){ gridMax.x, _planeWorldPos.y, gridMin.z }, 
        (Vector3){ gridMax.x, _planeWorldPos.y, gridMax.z }, 
        (Vector3){ gridMin.x, _planeWorldPos.y, gridMax.z });
    if (col.hit)
    {
        _cursor.endPosition = _mapMan.Map().SnapToCelCenter(col.point);
        _cursor.endPosition.y = _planeWorldPos.y + _mapMan.Map().GetSpacing() / 2.0f;
    }
    
    bool multiSelect = false;
    if (!_cursor.brushMode)
    {
        if (!IsKeyDown(KEY_LEFT_SHIFT))
        {
            _cursor.startPosition = _cursor.endPosition;
        }
        else
        {
            multiSelect = true;
        }
    }

    //Perform Tile operations
    Vector3 cursorStartGridPos = _mapMan.Map().WorldToGridPos(_cursor.startPosition);
    Vector3 cursorEndGridPos = _mapMan.Map().WorldToGridPos(_cursor.endPosition);
    Vector3 gridPosMin = Vector3Min(cursorStartGridPos, cursorEndGridPos);
    Vector3 gridPosMax = Vector3Max(cursorStartGridPos, cursorEndGridPos);
    size_t i = (size_t)gridPosMin.x; 
    size_t j = (size_t)gridPosMin.y;
    size_t k = (size_t)gridPosMin.z;
    size_t w = (size_t)gridPosMax.x - i + 1;
    size_t h = (size_t)gridPosMax.y - j + 1;
    size_t l = (size_t)gridPosMax.z - k + 1;
    Tile underTile = _mapMan.Map().GetTile(i, j, k); // * hi. my name's sans undertile...you get it?

    //Press Shift+B to enter brush mode, copying the currently selected rectangle of tiles.
    if (!_cursor.brushMode && IsKeyPressed(KEY_B) && IsKeyDown(KEY_LEFT_SHIFT))
    {
        _cursor.brushMode = true;
        _cursor.brush = _mapMan.Map().Subsection(i, j, k, w, h, l);
    }

    if (_cursor.brushMode)
    {
        _cursor.startPosition = (Vector3) {
            _cursor.endPosition.x + (_cursor.brush.GetWidth()  - 1) * _cursor.brush.GetSpacing(),
            _cursor.endPosition.y + (_cursor.brush.GetHeight() - 1) * _cursor.brush.GetSpacing(),
            _cursor.endPosition.z + (_cursor.brush.GetLength() - 1) * _cursor.brush.GetSpacing(),
        };
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            _mapMan.ExecuteTileAction(i, j, k, w, h, l, _cursor.brush);
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
        {
            _cursor.brushMode = false;
        }
    }
    else
    {
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !multiSelect) {
            //Place tiles
            if (underTile != _cursor.tile)
            {
                _mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, _cursor.tile);
            }
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && multiSelect)
        {
            //Place tiles rectangle
            _mapMan.ExecuteTileAction(i, j, k, w, h, l, _cursor.tile);
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !multiSelect) 
        {
            //Remove tiles
            if (underTile.shape != nullptr || underTile.texture != nullptr)
            {
                _mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, (Tile) {nullptr, ANGLE_0, nullptr, false});
            }
        } 
        else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && multiSelect)
        {
            //Remove tiles RECTANGLE
            _mapMan.ExecuteTileAction(i, j, k, w, h, l, (Tile) {nullptr, ANGLE_0, nullptr, false});
        }
        else if (IsKeyDown(KEY_G) && !multiSelect) 
        {
            //(G)rab the shape from the tile under the cursor
            if (underTile.shape) 
            {
                _cursor.tile.shape = underTile.shape;
                _cursor.tile.angle = underTile.angle;
            }
        } 
        else if (IsKeyDown(KEY_T) && !multiSelect) 
        {
            //Pick the (T)exture from the tile under the cursor.
            if (underTile.texture) 
            {
                _cursor.tile.texture = underTile.texture;
            }
        }
    }
}

void PlaceMode::Update() 
{
    MoveCamera();
    
    //Move editing plane
    if (GetMouseWheelMove() > EPSILON || GetMouseWheelMove() < -EPSILON) 
    {
        _planeGridPos.y = Clamp(_planeGridPos.y + Sign(GetMouseWheelMove()), 0.0f, _mapMan.Map().GetHeight() - 1);

        //Reveal hidden layers when the grid is over them and holding H
        if (IsKeyDown(KEY_H))
        {
            int planeLayer = (int)_planeGridPos.y;
            if (planeLayer < _layerViewMin) _layerViewMin = planeLayer;
            if (planeLayer > _layerViewMax) _layerViewMax = planeLayer;
        }
        else
        {
            //Prevent the user from editing hidden layers.
            _planeGridPos.y = Clamp(_planeGridPos.y, _layerViewMin, _layerViewMax);
        }
    }
    _planeWorldPos = _mapMan.Map().GridToWorldPos(_planeGridPos, false);

    //Layer hiding
    if (IsKeyPressed(KEY_H))
    {
        if (_layerViewMin == 0 && _layerViewMax == _mapMan.Map().GetHeight() - 1)
        {
            _layerViewMax = _layerViewMin = (int) _planeGridPos.y;
        }
        else
        {
            _layerViewMin = 0;
            _layerViewMax = _mapMan.Map().GetHeight() - 1;
        }
    }

    if (_layerViewMin > 0 || _layerViewMax < _mapMan.Map().GetHeight() - 1)
    {
        App::Get()->DisplayStatusMessage("PRESS H TO UNHIDE LAYERS", 0.25f, 1);
    }

    //Update cursor
    if (!CheckCollisionPointRec(GetMousePosition(), App::Get()->GetMenuBarRect())) //Don't update when using the menus
    {
        UpdateCursor();
    }

    //Undo and redo
    if (IsKeyDown(KEY_LEFT_CONTROL))
    {
        if (IsKeyPressed(KEY_Z)) _mapMan.Undo();
        else if (IsKeyPressed(KEY_Y)) _mapMan.Redo();
    }
}

void PlaceMode::Draw() 
{
    BeginMode3D(_camera);
    {
        //Grid
        rlDrawRenderBatchActive();
        rlSetLineWidth(1.0f);
        DrawGridEx(
            Vector3Add(_planeWorldPos, (Vector3){ 0.0f, 0.05f, 0.0f }), //Adding the offset to prevent Z-fighting 
            _mapMan.Map().GetWidth()+1, _mapMan.Map().GetLength()+1, 
            _mapMan.Map().GetSpacing());
        rlDrawRenderBatchActive();

        //Draw tiles
        _mapMan.DrawMap(Vector3Zero(), _layerViewMin, _layerViewMax);

        //Draw cursor
        if (!IsKeyDown(KEY_LEFT_SHIFT) && _cursor.tile.shape && _cursor.tile.texture) {
            Matrix cursorTransform = MatrixMultiply(
                TileRotationMatrix(_cursor.tile), 
                MatrixTranslate(_cursor.endPosition.x, _cursor.endPosition.y, _cursor.endPosition.z));
            
            if (_cursor.brushMode)
            {
                Vector3 brushOffset = Vector3Min(_cursor.startPosition, _cursor.endPosition);
                brushOffset.x -= _cursor.brush.GetSpacing() / 2.0f;
                brushOffset.y -= _cursor.brush.GetSpacing() / 2.0f;
                brushOffset.z -= _cursor.brush.GetSpacing() / 2.0f;
                _cursor.brush.Draw(brushOffset);
            }
            else
            {
                for (size_t m = 0; m < _cursor.tile.shape->meshCount; ++m) {
                    DrawMesh(_cursor.tile.shape->meshes[m], *Assets::GetMaterialForTexture(_cursor.tile.texture, false), cursorTransform);
                }
            }
        }
        rlDrawRenderBatchActive();
        rlDisableDepthTest();
        rlSetLineWidth(2.0f);
        DrawCubeWires(
            Vector3Scale(Vector3Add(_cursor.startPosition, _cursor.endPosition), 0.5f), 
            fabs(_cursor.endPosition.x - _cursor.startPosition.x) + _mapMan.Map().GetSpacing() * _cursor.outlineScale, 
            fabs(_cursor.endPosition.y - _cursor.startPosition.y) + _mapMan.Map().GetSpacing() * _cursor.outlineScale, 
            fabs(_cursor.endPosition.z - _cursor.startPosition.z) + _mapMan.Map().GetSpacing() * _cursor.outlineScale, 
            MAGENTA);

        DrawAxes3D((Vector3){ 1.0f, 1.0f, 1.0f }, 10.0f);
        rlDrawRenderBatchActive();
        rlEnableDepthTest();
    }
    EndMode3D();
}