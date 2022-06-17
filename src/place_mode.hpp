#ifndef PLACE_MODE_H
#define PLACE_MODE_H

#include "raylib.h"

#include <vector>
#include <string>
#include <deque>

#include "tile.hpp"
#include "app.hpp"
#include "menu_bar.hpp"

class PlaceMode : public App::ModeImpl {
public:
    enum class Mode { TILES, ENTS };
    
    PlaceMode(Mode mode);
    
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;

    inline void SetCursorShape(Model *shape) { _cursor.tile.shape = shape; }
    inline void SetCursorTexture(Texture2D *tex) { _cursor.tile.texture = tex; }

    void ResetCamera();
protected:
    struct TileAction {
        size_t i, j, k;
        TileGrid prevState;
        TileGrid newState;
    };

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
    
    //Queues a tile action for filling an area with one tile
    TileAction &QueueTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile);
    //Queues a tile action for filling an area using a brush
    TileAction &QueueTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, TileGrid brush);
    void DoAction(TileAction &action);
    void UndoAction(TileAction &action);

    Mode _mode;

    Camera _camera;
    float _cameraYaw;
    float _cameraPitch;
    float _cameraMoveSpeed;

    Cursor _cursor;
    TileGrid _tileGrid;
    int _layerViewMin, _layerViewMax;
    //Stores recently executed actions to be undone on command.
    std::deque<TileAction> _undoHistory;
    //Stores recently undone actions to be redone on command, unless the history is altered.
    std::deque<TileAction> _redoHistory;

    Vector3 _planeGridPos;
    Vector3 _planeWorldPos;
};

#endif