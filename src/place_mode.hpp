/*
Copyright (C) 2022 Alexander Lunsford
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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