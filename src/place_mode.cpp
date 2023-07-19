/**
 * Copyright (c) 2022 Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "place_mode.hpp"

#include "raymath.h"
#include "rlgl.h"
#include "extras/raygui.h"

#include "draw_extras.h"
#include "math_stuff.hpp"
#include "assets.hpp"

#include <iostream>

#define CAMERA_PITCH_LIMIT (PI / 2.5f)
#define CAMERA_MOVE_SPEED_MIN   8.0f
#define CAMERA_MOVE_SPEED_MAX   64.0f
#define CAMERA_ACCELERATION     16.0f

Tile PlaceMode::TileCursor::GetTile(MapMan& mapMan) const
{
    return Tile(
        mapMan.GetOrAddModelID(model->GetPath()),
        angle,
        mapMan.GetOrAddTexID(tex->GetPath()),
        pitch
    );
}

PlaceMode::PlaceMode(MapMan &mapMan) 
    : _mapMan(mapMan),
      _layerViewMax(mapMan.Tiles().GetHeight() - 1),
      _layerViewMin(0)
{
    //Setup camera
	_camera = { 0 };
	_camera.up = VEC3_UP;
	_camera.fovy = 70.0f;
	_camera.projection = CAMERA_PERSPECTIVE;
    _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

    //Setup cursors
    _tileCursor.tex = nullptr;
    _tileCursor.model = nullptr;
    _tileCursor.angle = _tileCursor.pitch = 0;

    _brushCursor.brush = TileGrid(&mapMan, 1, 1, 1);
    
    _entCursor.ent = Ent(1.0f);
    _entCursor.ent.properties["name"] = "entity";

    _cursor = &_tileCursor;
    _cursor->position = _tileCursor.endPosition = Vector3Zero();
    _outlineScale = 1.125f;

    _cursorMaterial = LoadMaterialDefault();
    _cursorMaterial.shader = Assets::GetMapShader(false);

    ResetCamera();
    ResetGrid();
}

PlaceMode::~PlaceMode() 
{
    //Free the cursor material
    //Do not use UnloadMaterial, because it will free the texture that the material uses, which might be being used somewhere else.
    RL_FREE(_cursorMaterial.maps);
}

void PlaceMode::SetCursorShape(std::shared_ptr<Assets::ModelHandle> shape) 
{ 
    _tileCursor.model = shape;
    _cursor = &_tileCursor;
}

void PlaceMode::SetCursorTexture(std::shared_ptr<Assets::TexHandle> tex)
{ 
    _tileCursor.tex = tex; 
    _cursor = &_tileCursor;
}

void PlaceMode::SetCursorEnt(const Ent &ent) 
{ 
    _entCursor.ent = ent; 
    _cursor = &_entCursor; 
}

std::shared_ptr<Assets::ModelHandle> PlaceMode::GetCursorShape() const 
{ 
    return _tileCursor.model; 
}

std::shared_ptr<Assets::TexHandle> PlaceMode::GetCursorTexture() const 
{ 
    return _tileCursor.tex; 
}

const Ent& PlaceMode::GetCursorEnt() const 
{
    return _entCursor.ent; 
}

void PlaceMode::OnEnter() 
{
    if (_tileCursor.tex == nullptr || _tileCursor.model == nullptr)
    {
        _tileCursor.tex = Assets::GetTexture(App::Get()->GetDefaultTexturePath());
        _tileCursor.model = Assets::GetModel(App::Get()->GetDefaultShapePath());
    }
}

void PlaceMode::OnExit() 
{
}

void PlaceMode::ResetCamera()
{
    _camera.position = _mapMan.Tiles().GetCenterPos();
    _camera.target = _camera.position;
    _cameraYaw = 0.0f;
    _cameraPitch = -PI / 4.0f;
    _cursor->position = _tileCursor.endPosition = Vector3Zero();
}

void PlaceMode::ResetGrid()
{
    //Editor grid and plane
    _planeGridPos = Vector3{ (float)_mapMan.Tiles().GetWidth() / 2.0f, 0, (float)_mapMan.Tiles().GetLength() / 2.0f };
    _planeWorldPos = _mapMan.Tiles().GridToWorldPos(_planeGridPos, false);
    _layerViewMin = 0;
    _layerViewMax = _mapMan.Tiles().GetHeight() - 1;
    //Cursor
    _cursor->position = _tileCursor.endPosition = Vector3Zero();
    _cursor = &_tileCursor;
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
    
    if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_ALT))) 
    {
        _cameraYaw -= GetMouseDelta().x * App::Get()->GetMouseSensitivity() * GetFrameTime();
        _cameraPitch -= GetMouseDelta().y * App::Get()->GetMouseSensitivity() * GetFrameTime();
        _cameraPitch = Clamp(_cameraPitch, -CAMERA_PITCH_LIMIT, CAMERA_PITCH_LIMIT);
    }

    cameraMovement = Vector3Scale(Vector3Normalize(cameraMovement), _cameraMoveSpeed * GetFrameTime());
    Matrix cameraRotation = MatrixMultiply(MatrixRotateX(_cameraPitch), MatrixRotateY(_cameraYaw));
    Vector3 rotatedMovement = Vector3Transform(cameraMovement, cameraRotation);
    
    _camera.position = Vector3Add(_camera.position, rotatedMovement);
    _camera.target = Vector3Add(_camera.position, Vector3Transform(VEC3_FORWARD, cameraRotation));
}

//TODO: Separate cursor updates into virtual functions
void PlaceMode::UpdateCursor()
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE))
    {
        _cursor = &_tileCursor;
    }
    else if (IsKeyPressed(KEY_E) && IsKeyDown(KEY_LEFT_CONTROL))
    {
        _cursor = &_entCursor;
    }

    //Position cursor
    Ray pickRay = GetMouseRay(GetMousePosition(), _camera);
    Vector3 gridMin = _mapMan.Tiles().GetMinCorner();
    Vector3 gridMax = _mapMan.Tiles().GetMaxCorner();
    RayCollision col = GetRayCollisionQuad(pickRay, 
        Vector3{ gridMin.x, _planeWorldPos.y, gridMin.z }, 
        Vector3{ gridMax.x, _planeWorldPos.y, gridMin.z }, 
        Vector3{ gridMax.x, _planeWorldPos.y, gridMax.z }, 
        Vector3{ gridMin.x, _planeWorldPos.y, gridMax.z });
    if (col.hit)
    {
        _cursor->position = _mapMan.Tiles().SnapToCelCenter(col.point);
        _cursor->position.y = _planeWorldPos.y + _mapMan.Tiles().GetSpacing() / 2.0f;
    }
    
    //Handle box selection/fill in tile mode.
    bool multiSelect = false;
    if (_cursor == &_tileCursor)
    {
        if (!IsKeyDown(KEY_LEFT_SHIFT))
        {
            _tileCursor.endPosition = _tileCursor.position;
        }
        else
        {
            multiSelect = true;
        }
    }

    //Perform Tile operations
    Vector3 cursorStartGridPos = _mapMan.Tiles().WorldToGridPos(_cursor->position);
    Vector3 cursorEndGridPos = _mapMan.Tiles().WorldToGridPos(_cursor->endPosition);
    Vector3 gridPosMin = Vector3Min(cursorStartGridPos, cursorEndGridPos);
    Vector3 gridPosMax = Vector3Max(cursorStartGridPos, cursorEndGridPos);
    size_t i = (size_t)gridPosMin.x; 
    size_t j = (size_t)gridPosMin.y;
    size_t k = (size_t)gridPosMin.z;
    size_t w = (size_t)gridPosMax.x - i + 1;
    size_t h = (size_t)gridPosMax.y - j + 1;
    size_t l = (size_t)gridPosMax.z - k + 1;
    Tile underTile = _mapMan.Tiles().GetTile(i, j, k); // * hi. my name's sans undertile...you get it?

    //Press Shift+B to enter brush mode, copying the currently selected rectangle of tiles.
    if (_cursor == &_tileCursor && IsKeyPressed(KEY_B) && IsKeyDown(KEY_LEFT_SHIFT))
    {
        _cursor = &_brushCursor;
        _brushCursor.brush = _mapMan.Tiles().Subsection(i, j, k, w, h, l);
        _brushCursor.position = _tileCursor.position;
        _brushCursor.endPosition = _tileCursor.endPosition;
    }

    if (_cursor == &_brushCursor)
    {
        //Put the end position at the other extent of the bounding box so that a border can be drawn later
        _brushCursor.endPosition = Vector3 {
            _brushCursor.position.x + ((_brushCursor.brush.GetWidth() - 1) * _brushCursor.brush.GetSpacing()),
            _brushCursor.position.y + ((_brushCursor.brush.GetHeight() - 1) * _brushCursor.brush.GetSpacing()),
            _brushCursor.position.z + ((_brushCursor.brush.GetLength() - 1) * _brushCursor.brush.GetSpacing()),
        };
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_ALT))
        {
            _mapMan.ExecuteTileAction(i, j, k, w, h, l, _brushCursor.brush);
        }
    }
    else if (_cursor == &_tileCursor)
    {
        //Rotate cursor
        if (IsKeyPressed(KEY_Q))
        {
            _tileCursor.angle = OffsetDegrees(_tileCursor.angle, -90);
        }
        else if (IsKeyPressed(KEY_E))
        {
            _tileCursor.angle = OffsetDegrees(_tileCursor.angle, 90);
        }
        if (IsKeyPressed(KEY_F))
        {
            _tileCursor.pitch = OffsetDegrees(_tileCursor.pitch, 90);
        }
        else if (IsKeyPressed(KEY_V))
        {
            _tileCursor.pitch = OffsetDegrees(_tileCursor.pitch, -90);
        }
        //Reset tile orientation
        if (IsKeyPressed(KEY_R))
        {
            _tileCursor.angle = _tileCursor.pitch = 0;
        }

        Tile cursorTile = _tileCursor.GetTile(_mapMan);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_ALT) && !multiSelect) 
        {
            //Place tiles
            if (underTile != cursorTile)
            {
                _mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, cursorTile);
            }
        }
        else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && multiSelect)
        {
            //Place tiles rectangle
            _mapMan.ExecuteTileAction(i, j, k, w, h, l, cursorTile);
        }
        else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !multiSelect) 
        {
            //Remove tiles
            if (underTile)
            {
                _mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, Tile {NO_MODEL, 0, NO_TEX, false});
            }
        } 
        else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && multiSelect)
        {
            //Remove tiles RECTANGLE
            _mapMan.ExecuteTileAction(i, j, k, w, h, l, Tile {NO_MODEL, 0, NO_TEX, false});
        }
        else if (IsKeyPressed(KEY_G) && !multiSelect) 
        {
            //(G)rab the shape from the tile under the cursor
            if (underTile) 
            {
                std::cout << "ID: " << underTile.shape << std::endl;
                auto path = _mapMan.PathFromModelID(underTile.shape);
                std::cout << "PATH: " << path << std::endl;
                _tileCursor.model = Assets::GetModel(path);
                _tileCursor.angle = underTile.angle;
                _tileCursor.pitch = underTile.pitch;
            }
        } 
        else if (IsKeyPressed(KEY_T) && !multiSelect) 
        {
            //Pick the (T)exture from the tile under the cursor.
            if (underTile) 
            {
                std::cout << "ID: " << underTile.texture << std::endl;
                auto path = _mapMan.PathFromTexID(underTile.texture);
                std::cout << "PATH: " << path << std::endl;
                _tileCursor.tex = Assets::GetTexture(path);
            }
        }
    }
    else if (_cursor == &_entCursor)
    {
        _entCursor.endPosition = _entCursor.position;
        _entCursor.ent.position = _entCursor.position;

        //Turn entity
        const int ANGLE_INC = IsKeyDown(KEY_LEFT_SHIFT) ? 15 : 45;
        if (IsKeyPressed(KEY_Q))
        {
            _entCursor.ent.yaw = OffsetDegrees(_entCursor.ent.yaw, -ANGLE_INC);
        }
        else if (IsKeyPressed(KEY_E))
        {
            _entCursor.ent.yaw = OffsetDegrees(_entCursor.ent.yaw, ANGLE_INC);
        }
        if (IsKeyPressed(KEY_F))
        {
            _entCursor.ent.pitch = OffsetDegrees(_entCursor.ent.pitch, ANGLE_INC);
        }
        else if (IsKeyPressed(KEY_V))
        {
            _entCursor.ent.pitch = OffsetDegrees(_entCursor.ent.pitch, -ANGLE_INC);
        }

        //Reset ent orientation
        if (IsKeyPressed(KEY_R))
        {
            _entCursor.ent.yaw = _entCursor.ent.pitch = 0;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_ALT))
        {
            //Placement
            _mapMan.ExecuteEntPlacement(i, j, k, _entCursor.ent);
        }
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
        {
            //Removal
            if (_mapMan.Ents().HasEnt(i, j, k))
            {
                _mapMan.ExecuteEntRemoval(i, j, k);
            }
        }

        if (IsKeyPressed(KEY_T) || IsKeyPressed(KEY_G))
        {
            //Copy the entity under the cursor
            if (_mapMan.Ents().HasEnt(i, j, k))
            {
                _entCursor.ent = _mapMan.Ents().GetEnt(i, j, k);
            }
        }
    }
}

void PlaceMode::Update() 
{
    MoveCamera();

    if (!App::Get()->IsPreviewing())
    {    
        //Move editing plane
        if (GetMouseWheelMove() > EPSILON || GetMouseWheelMove() < -EPSILON) 
        {
            _planeGridPos.y = Clamp(_planeGridPos.y + Sign(GetMouseWheelMove()), 0.0f, _mapMan.Tiles().GetHeight() - 1);

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
        _planeWorldPos = _mapMan.Tiles().GridToWorldPos(_planeGridPos, false);

        //Layer hiding
        if (IsKeyPressed(KEY_H))
        {
            if (_layerViewMin == 0 && _layerViewMax == _mapMan.Tiles().GetHeight() - 1)
            {
                _layerViewMax = _layerViewMin = (int) _planeGridPos.y;
            }
            else
            {
                _layerViewMin = 0;
                _layerViewMax = _mapMan.Tiles().GetHeight() - 1;
            }
        }

        if (_layerViewMin > 0 || _layerViewMax < _mapMan.Tiles().GetHeight() - 1)
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
}

void PlaceMode::Draw() 
{
    BeginMode3D(_camera);
    {
        //Draw map
        _mapMan.DrawMap(_camera, _layerViewMin, _layerViewMax);

        if (!App::Get()->IsPreviewing())
        {
            //Grid
            rlDrawRenderBatchActive();
            rlSetLineWidth(1.0f);
            DrawGridEx(
                Vector3Add(_planeWorldPos, Vector3{ 0.0f, 0.05f, 0.0f }), //Adding the offset to prevent Z-fighting 
                _mapMan.Tiles().GetWidth()+1, _mapMan.Tiles().GetLength()+1, 
                _mapMan.Tiles().GetSpacing());
            rlDrawRenderBatchActive();

            //Draw cursor
            if (_cursor == &_tileCursor)
            {
                if (!IsKeyDown(KEY_LEFT_SHIFT))
                {
                    Matrix cursorTransform = MatrixMultiply(
                        TileRotationMatrix(_tileCursor.GetTile(_mapMan)), 
                        MatrixTranslate(_cursor->position.x, _cursor->position.y, _cursor->position.z));
                    
                    _cursorMaterial.maps[MATERIAL_MAP_ALBEDO].texture = _tileCursor.tex->GetTexture();
                    const Model &shape = _tileCursor.model->GetModel();
                    for (size_t m = 0; m < shape.meshCount; ++m) 
                    {
                        DrawMesh(shape.meshes[m], _cursorMaterial, cursorTransform);
                    }
                }
            }
            else if (_cursor == &_brushCursor)
            {
                // Draw the tile grid within the brush
                Vector3 brushOffset = Vector3Min(_brushCursor.position, _brushCursor.endPosition);
                brushOffset.x -= _brushCursor.brush.GetSpacing() / 2.0f;
                brushOffset.y -= _brushCursor.brush.GetSpacing() / 2.0f;
                brushOffset.z -= _brushCursor.brush.GetSpacing() / 2.0f;
                _brushCursor.brush.Draw(brushOffset);
            }
            else if (_cursor == &_entCursor)
            {
                _entCursor.ent.Draw();
            }

            // Draw pink border around the cursor
            rlDrawRenderBatchActive();
            rlDisableDepthTest();
            rlSetLineWidth(2.0f);
            DrawCubeWires(
                Vector3Scale(Vector3Add(_cursor->position, _cursor->endPosition), 0.5f), 
                fabs(_cursor->endPosition.x - _cursor->position.x) + _mapMan.Tiles().GetSpacing() * _outlineScale, 
                fabs(_cursor->endPosition.y - _cursor->position.y) + _mapMan.Tiles().GetSpacing() * _outlineScale, 
                fabs(_cursor->endPosition.z - _cursor->position.z) + _mapMan.Tiles().GetSpacing() * _outlineScale, 
                MAGENTA);

            DrawAxes3D(Vector3{ 1.0f, 1.0f, 1.0f }, 10.0f);
            rlDrawRenderBatchActive();
            rlEnableDepthTest();
        }
    }
    EndMode3D();

    _mapMan.Draw2DElements(_camera, _layerViewMin, _layerViewMax);
}
