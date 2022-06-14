#ifndef PLACE_MODE_H
#define PLACE_MODE_H

#include "raylib.h"

#include <vector>
#include <string>
#include <deque>

#include "editor_mode.hpp"
#include "tile.hpp"
#include "app.hpp"
#include "menu_bar.hpp"

class PlaceMode : public EditorMode {
public:
    PlaceMode(AppContext *context);
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
protected:
    struct TileAction {
        size_t i, j, k;
        TileGrid prevState;
        TileGrid newState;
    };

    struct Cursor {
        Tile tile;
        Vector3 endPosition;
        Vector3 startPosition;
        float outlineScale;
    };

    void MoveCamera();
    void UpdateCursor();
    TileAction &QueueTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile);
    void DoAction(TileAction &action);
    void UndoAction(TileAction &action);

    AppContext *_context;

    Camera _camera;
    float _cameraYaw;
    float _cameraPitch;
    float _cameraMoveSpeed;

    MenuBar _menuBar;

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