/**
 * Copyright (c) 2022-present Alexander Lunsford
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
#include "imgui/imgui.h"

#include "../draw_extras.h"
#include "../math_stuff.hpp"
#include "../assets.hpp"

#include <iostream>

#define CAMERA_PITCH_LIMIT (PI / 2.5f)
#define CAMERA_MOVE_SPEED_MIN 8.0f
#define CAMERA_MOVE_SPEED_MAX 64.0f
#define CAMERA_ACCELERATION   16.0f

PlaceMode::PlaceMode(MapMan &mapMan) 
    : _mapMan(mapMan),
      _brushCursor(mapMan),
      _layerViewMin(0),
      _layerViewMax(mapMan.Tiles().GetHeight() - 1)
{
    //Setup camera
	_camera = { .up = VEC3_UP, .fovy = 70.0f, .projection = CAMERA_PERSPECTIVE };
    _cameraMoveSpeed = CAMERA_MOVE_SPEED_MIN;

    _cursor = &_tileCursor;
    _cursor->position = _tileCursor.endPosition = Vector3Zero();
    _outlineScale = 1.125f;

    ResetCamera();
    ResetGrid();
}

void PlaceMode::SetCursorShape(std::shared_ptr<Assets::ModelHandle> shape) 
{ 
    _tileCursor.model = shape;
    _cursor = &_tileCursor;
}

void PlaceMode::SetCursorTextures(std::array<std::shared_ptr<Assets::TexHandle>, TEXTURES_PER_TILE> textures)
{ 
    _tileCursor.textures = textures; 
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

std::array<std::shared_ptr<Assets::TexHandle>, TEXTURES_PER_TILE> PlaceMode::GetCursorTextures() const 
{ 
    return _tileCursor.textures; 
}

const Ent& PlaceMode::GetCursorEnt() const 
{
    return _entCursor.ent;
}

void PlaceMode::OnEnter() 
{
    if (_tileCursor.model == nullptr)
    {
        for (auto& texture : _tileCursor.textures) 
        {
            texture = Assets::GetTexture(App::Get()->GetDefaultTexturePath());
        }
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

Vector3 PlaceMode::GetCameraPosition() const
{
    return _camera.position;
}

Vector3 PlaceMode::GetCameraAngles() const
{
    return Vector3 { _cameraPitch, _cameraYaw, 0.0f };
}

void PlaceMode::SetCameraOrientation(Vector3 position, Vector3 angles)
{
    _camera.position = position;
    _cameraPitch = angles.x;
    _cameraYaw = angles.y;
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

    cameraMovement = Vector3Normalize(cameraMovement) * _cameraMoveSpeed * GetFrameTime();
    Matrix cameraRotation = MatrixRotateX(_cameraPitch) * MatrixRotateY(_cameraYaw);
    Vector3 rotatedMovement = cameraMovement * cameraRotation;
    
    _camera.position = _camera.position + rotatedMovement;
    _camera.target = _camera.position + (VEC3_FORWARD * cameraRotation);

    // Set the default camera orientation when the map is saved
    _mapMan.SetDefaultCameraPosition(_camera.position);
    _mapMan.SetDefaultCameraAngles(Vector3 { _cameraPitch, _cameraYaw, 0.0f });
}

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

    // Position cursor
    Vector2 currentMousePosition = GetMousePosition();
    if (Vector2LengthSqr(currentMousePosition - _previousMousePosition) > 1.0f) 
    {
        Ray pickRay = GetMouseRay(currentMousePosition, _camera);
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
    }
    else 
    {
        // When the mouse is not moving, the cursor should stay in the same position laterally but move up and down with the editor plane.
        _cursor->position.y = _planeWorldPos.y + _mapMan.Tiles().GetSpacing() / 2.0f;
    }
    _previousMousePosition = currentMousePosition;

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
    
    if (_cursorPreviousGridPos != cursorStartGridPos) 
    {
        std::stringstream cursorPosition;
        cursorPosition << "x: " << cursorStartGridPos.x << " y: " << cursorStartGridPos.y << " z: " << cursorStartGridPos.z;
        App::Get()->DisplayStatusMessage(cursorPosition.str().c_str(), 1.0f, 2);
        _cursorPreviousGridPos = cursorStartGridPos;
    }

    // Press Shift+B to enter brush mode, copying the currently selected rectangle of tiles.
    if (_cursor == &_tileCursor && IsKeyPressed(KEY_B) && IsKeyDown(KEY_LEFT_SHIFT))
    {
        _cursor = &_brushCursor;
        _brushCursor.brush = _mapMan.Tiles().Subsection(i, j, k, w, h, l);
        _brushCursor.position = _tileCursor.position;
        _brushCursor.endPosition = _tileCursor.endPosition;
    }

    _cursor->Update(_mapMan, i, j, k, w, h, l);
}

void PlaceMode::Update() 
{
    // Don't update this when using the GUI
    if (auto io = ImGui::GetIO(); io.WantCaptureMouse || io.WantCaptureKeyboard) 
    {
        return;
    }

    MoveCamera();

    if (!App::Get()->IsPreviewing())
    {    
        // Move editing plane
        if (GetMouseWheelMove() > EPSILON || GetMouseWheelMove() < -EPSILON) 
        {
            _planeGridPos.y = Clamp(_planeGridPos.y + Sign(GetMouseWheelMove()), 0.0f, _mapMan.Tiles().GetHeight() - 1);

            // Reveal hidden layers when the grid is over them and holding H
            if (IsKeyDown(KEY_H))
            {
                int planeLayer = (int)_planeGridPos.y;
                if (planeLayer < _layerViewMin) _layerViewMin = planeLayer;
                if (planeLayer > _layerViewMax) _layerViewMax = planeLayer;
            }
            else
            {
                // Prevent the user from editing hidden layers.
                _planeGridPos.y = Clamp(_planeGridPos.y, _layerViewMin, _layerViewMax);
            }
        }
        _planeWorldPos = _mapMan.Tiles().GridToWorldPos(_planeGridPos, false);

        // Layer hiding
        if (IsKeyPressed(KEY_H))
        {
            if (_layerViewMin == 0 && (size_t)_layerViewMax == _mapMan.Tiles().GetHeight() - 1)
            {
                _layerViewMax = _layerViewMin = (int) _planeGridPos.y;
            }
            else
            {
                _layerViewMin = 0;
                _layerViewMax = _mapMan.Tiles().GetHeight() - 1;
            }
        }

        if (_layerViewMin > 0 || (size_t)_layerViewMax < _mapMan.Tiles().GetHeight() - 1)
        {
            App::Get()->DisplayStatusMessage("PRESS H TO UNHIDE LAYERS", 0.25f, 1);
        }

        // Update cursor
        UpdateCursor();

        // Undo and redo
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
        // Draw map
        if (!App::Get()->IsPreviewing())
        {
            // Grid
            rlDrawRenderBatchActive();
            rlSetLineWidth(1.0f);
            DrawGridEx(
                _planeWorldPos + Vector3{ 0.0f, 0.05f, 0.0f }, // Adding the offset to prevent Z-fighting 
                _mapMan.Tiles().GetWidth() + 1, _mapMan.Tiles().GetLength() + 1, 
                _mapMan.Tiles().GetSpacing());
            // We need to draw the grid BEFORE the map, or there will be visual errors with entity billboards
            rlDrawRenderBatchActive();
        }
        
        _mapMan.DrawMap(_camera, _layerViewMin, _layerViewMax);

        if (!App::Get()->IsPreviewing())
        {
            //Draw cursor
            _cursor->Draw();

            // Draw pink border around the cursor
            rlDrawRenderBatchActive();
            rlDisableDepthTest();
            rlSetLineWidth(2.0f);
            DrawCubeWires(
                (_cursor->position + _cursor->endPosition) * 0.5f, 
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
