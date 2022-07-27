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

#ifndef PLACE_MODE_H
#define PLACE_MODE_H

#include "raylib.h"

#include <vector>
#include <string>

#include "tile.hpp"
#include "ent.hpp"
#include "app.hpp"
#include "menu_bar.hpp"
#include "map_man.hpp"

class PlaceMode : public App::ModeImpl {
public:
    PlaceMode(MapMan &mapMan);

    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    inline void SetCursorShape(ModelID shape) { _cursor.tile.shape = shape; _cursor.mode = Cursor::Mode::TILE; }
    inline void SetCursorTexture(TexID tex) { _cursor.tile.texture = tex; _cursor.mode = Cursor::Mode::TILE; }
    inline void SetCursorEnt(const Ent &ent) { _cursor.ent = ent; _cursor.mode = Cursor::Mode::ENT; }
    inline ModelID GetCursorShape() const { return _cursor.tile.shape; }
    inline TexID GetCursorTexture() const { return _cursor.tile.texture; }
    inline const Ent &GetCursorEnt() const { return _cursor.ent; }

    void ResetCamera();
    void ResetGrid();
protected:
    struct Cursor {
        Tile tile;
        TileGrid brush;
        Ent ent;
        Vector3 endPosition;
        Vector3 startPosition;
        float outlineScale;
        
        enum class Mode { TILE, BRUSH, ENT };
        Mode mode;
    };

    void MoveCamera();
    void UpdateCursor();

    MapMan &_mapMan;

    Camera _camera;
    float _cameraYaw;
    float _cameraPitch;
    float _cameraMoveSpeed;

    Cursor _cursor;
    int _layerViewMin, _layerViewMax;

    Vector3 _planeGridPos;
    Vector3 _planeWorldPos;
};

#endif
