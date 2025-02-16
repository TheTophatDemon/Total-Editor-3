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

#ifndef PLACE_MODE_H
#define PLACE_MODE_H

#include "raylib.h"

#include <vector>
#include <string>
#include <array>

#include "../tile.hpp"
#include "../ent.hpp"
#include "../app.hpp"
#include "../menu_bar.hpp"
#include "../map_man.hpp"

class PlaceMode : public App::ModeImpl {
public:
    PlaceMode(MapMan &mapMan);

    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    void SetCursorShape(std::shared_ptr<Assets::ModelHandle> shape);
    void SetCursorTextures(std::array<std::shared_ptr<Assets::TexHandle>, TEXTURES_PER_TILE> tex);
    void SetCursorEnt(const Ent &ent);
    std::shared_ptr<Assets::ModelHandle> GetCursorShape() const;
    std::array<std::shared_ptr<Assets::TexHandle>, TEXTURES_PER_TILE> GetCursorTextures() const;
    const Ent &GetCursorEnt() const;

    void ResetCamera();
    Vector3 GetCameraPosition() const;
    Vector3 GetCameraAngles() const;
    void SetCameraOrientation(Vector3 position, Vector3 angles);
    void ResetGrid();
protected:
    struct Cursor 
    {
        Vector3 position;
        Vector3 endPosition;

        virtual void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) = 0;
        virtual void Draw() = 0;
    };

    struct TileCursor : public Cursor
    {
        std::shared_ptr<Assets::ModelHandle> model;
        std::array<std::shared_ptr<Assets::TexHandle>, TEXTURES_PER_TILE> textures;
        std::array<Material, TEXTURES_PER_TILE> materials;
        uint8_t yaw, pitch;

        TileCursor();
        ~TileCursor();
        void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) override;
        void Draw() override;
    };

    struct BrushCursor : public Cursor
    {
        TileGrid brush;

        BrushCursor(MapMan& mapMan);
        void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) override;
        void Draw() override;
    };

    struct EntCursor : public Cursor
    {
        Ent ent;

        EntCursor();
        void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) override;
        void Draw() override;
    };

    void MoveCamera();
    void UpdateCursor();

    MapMan& _mapMan;

    Camera _camera;
    float _cameraYaw;
    float _cameraPitch;
    float _cameraMoveSpeed;

    TileCursor _tileCursor;
    BrushCursor _brushCursor;
    EntCursor _entCursor;
    Cursor* _cursor; // The current cursor being used in the editor. Will point to one of: _tileCursor, _brushCursor, _entCursor
    Vector3 _cursorPreviousGridPos;
    Vector2 _previousMousePosition;

    float _outlineScale; // How much the wire box around the cursor is larger than its contents

    int _layerViewMin, _layerViewMax;

    Vector3 _planeGridPos;
    Vector3 _planeWorldPos;
};

#endif
