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
    ~PlaceMode();

    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    inline void SetCursorShape(std::shared_ptr<Assets::ModelHandle> shape) 
    { 
        _tileCursor.model = shape;
        _cursor = &_tileCursor;
    }

    inline void SetCursorTexture(std::shared_ptr<Assets::TexHandle> tex)
    { 
        _tileCursor.tex = tex; 
        _cursor = &_tileCursor;
    }

    inline void SetCursorEnt(const Ent &ent) 
    { 
        _entCursor.ent = ent; 
        _cursor = &_entCursor; 
    }

    inline std::shared_ptr<Assets::ModelHandle> GetCursorShape() const 
    { 
        return _tileCursor.model; 
    }
    
    inline std::shared_ptr<Assets::TexHandle> GetCursorTexture() const 
    { 
        return _tileCursor.tex; 
    }
    
    inline const Ent &GetCursorEnt() const 
    {
        return _entCursor.ent; 
    }

    void ResetCamera();
    void ResetGrid();
protected:
    struct Cursor 
    {
        Vector3 position;
        Vector3 endPosition;
        float outlineScale;
    };

    struct TileCursor : public Cursor
    {
        std::shared_ptr<Assets::ModelHandle> model;
        std::shared_ptr<Assets::TexHandle> tex;
        int angle, pitch;

        inline Tile GetTile(MapMan& mapMan) const
        {
            return Tile(
                mapMan.GetOrAddModelID(model->GetPath()),
                angle,
                mapMan.GetOrAddTexID(tex->GetPath()),
                pitch
            );
        }
    };

    struct BrushCursor : public Cursor
    {
        TileGrid brush;
    };

    struct EntCursor : public Cursor
    {
        Ent ent;
    };

    void MoveCamera();
    void UpdateCursor();

    MapMan& _mapMan;

    Camera _camera;
    float _cameraYaw;
    float _cameraPitch;
    float _cameraMoveSpeed;

    Cursor* _cursor; //The current cursor being used in the editor. Will point to one of: _tileCursor, _brushCursor, _entCursor
    TileCursor _tileCursor;
    BrushCursor _brushCursor;
    EntCursor _entCursor;

    Material _cursorMaterial; //Material used to render the cursor

    int _layerViewMin, _layerViewMax;

    Vector3 _planeGridPos;
    Vector3 _planeWorldPos;
};

#endif
