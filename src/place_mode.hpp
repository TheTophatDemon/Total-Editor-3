#ifndef PLACE_MODE_H
#define PLACE_MODE_H

#include "raylib.h"

#include <vector>
#include <string>

#include "editor_mode.hpp"
#include "tile.hpp"
#include "app.hpp"

class PlaceMode : public EditorMode {
public:
    PlaceMode(AppContext *context);
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
protected:
    struct Cursor {
        Model *shape;
        Texture2D *texture;
        Angle angle;
        Vector3 position;
        float outlineScale;
    };

    void MoveCamera();

    AppContext *_context;

    Camera _camera;
    float _cameraYaw;
    float _cameraMoveSpeed;

    Cursor _cursor;
    TileGrid _tileGrid;

    Vector3 _planeGridPos;
    Vector3 _planeWorldPos;
};

#endif