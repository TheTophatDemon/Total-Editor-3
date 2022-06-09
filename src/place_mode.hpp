#ifndef PLACE_MODE_H
#define PLACE_MODE_H

#include "raylib.h"

#include <vector>

#include "editor_mode.hpp"
#include "tile.hpp"

class PlaceMode : public EditorMode {
public:
    PlaceMode();
    virtual void Update() override;
    virtual void Draw() override;
protected:
    struct Cursor {
        size_t shapeIndex;
        size_t materialIndex;
        Angle angle;
        Vector3 position;
        float outlineScale;
    };

    void MoveCamera();

    std::vector<Material> _instancedMaterials;
    Shader _mapShader;

    Camera _camera;
    float _cameraYaw;
    float _cameraMoveSpeed;

    Cursor _cursor;
    TileGrid _tileGrid;

    Vector3 _planeGridPos;
    Vector3 _planeWorldPos;
};

#endif