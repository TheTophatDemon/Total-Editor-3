#ifndef PLACE_MODE_H
#define PLACE_MODE_H

#include "raylib.h"

#include <vector>
#include <string>

#include "tile.hpp"
#include "app.hpp"
#include "menu_bar.hpp"
#include "map_man.hpp"

class PlaceMode : public App::ModeImpl {
public:
    enum class Mode { TILES, ENTS };
    
    PlaceMode(MapMan &mapMan, Mode mode);

    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    inline void SetCursorShape(Model *shape) { _cursor.tile.shape = shape; }
    inline void SetCursorTexture(Texture2D *tex) { _cursor.tile.texture = tex; }

    void ResetCamera();
    void ResetGrid();
protected:
    struct Cursor {
        Tile tile;
        TileGrid brush;
        Vector3 endPosition;
        Vector3 startPosition;
        float outlineScale;
        bool brushMode;
    };

    void MoveCamera();
    void UpdateCursor();

    MapMan &_mapMan;
    Mode _mode;

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