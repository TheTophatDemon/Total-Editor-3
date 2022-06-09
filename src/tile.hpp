#ifndef TILE_H
#define TILE_H

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>
#include <vector>

typedef enum Angle { ANGLE_0, ANGLE_90, ANGLE_180, ANGLE_270 } Angle;

float AngleDegrees(Angle angle);
float AngleRadians(Angle angle);
Angle AngleBack(Angle angle);
Angle AngleForward(Angle angle);

typedef struct Tile {
    Model* shape;
    Angle angle;
    Material* material;
} Tile;

class TileGrid {
public:
    TileGrid(size_t width, size_t height, size_t length, float spacing);

    inline Vector3 WorldToGridPos(Vector3 worldPos) const {
        return Vector3{ floorf(worldPos.x / _spacing) + (_width / 2), floorf(worldPos.y / _spacing) + (_height / 2), floorf(worldPos.z / _spacing) + (_length / 2) };
    }

    //Converts (whole number) grid cel coordinates to world coordinates.
    //If `center` is true, then the world coordinate will be in the center of the cel instead of the corner.
    inline Vector3 GridToWorldPos(Vector3 gridPos, bool center) const {
        if (center) {
            return Vector3{
                (gridPos.x * _spacing) + (_spacing / 2.0f) - (_width * _spacing / 2.0f),
                (gridPos.y * _spacing) + (_spacing / 2.0f) - (_height * _spacing / 2.0f),
                (gridPos.z * _spacing) + (_spacing / 2.0f) - (_length * _spacing / 2.0f),
            };
        } else {
            return Vector3{
                (gridPos.x * _spacing) - (_width * _spacing / 2.0f),
                (gridPos.y * _spacing) - (_height * _spacing / 2.0f),
                (gridPos.z * _spacing) - (_length * _spacing / 2.0f),
            };
        }
    }

    inline Vector3 SnapToCelCenter(Vector3 worldPos) const {
        worldPos.x = (floorf(worldPos.x / _spacing) * _spacing) + (_spacing / 2.0f);
        worldPos.y = (floorf(worldPos.y / _spacing) * _spacing) + (_spacing / 2.0f);
        worldPos.z = (floorf(worldPos.z / _spacing) * _spacing) + (_spacing / 2.0f);
        return worldPos;
    }

    inline size_t FlatIndex(int i, int j, int k) {
        return i + (k * _width) + (j * _width * _length);
    }

    inline Vector3 UnflattenIndex(size_t idx) const {
        return Vector3{
            (float)(idx % _width),
            (float)(idx / (_width * _length)),
            (float)((idx / _width) % _length)
        };
    }

    inline void SetTile(int i, int j, int k, const Tile& tile) {
        _grid[FlatIndex(i, j, k)] = tile;
    }

    inline void UnsetTile(int i, int j, int k) {
        _grid[FlatIndex(i, j, k)].shape = nullptr;
    }

    inline size_t GetWidth() const { return _width; }
    inline size_t GetHeight() const { return _height; }
    inline size_t GetLength() const { return _length; }
    inline float GetSpacing() const { return _spacing; }

    inline Vector3 GetMinCorner() const {
        return Vector3{ -((int)_width / 2) * _spacing, -((int)_height / 2) * _spacing, -((int)_length / 2) * _spacing };
    }
    inline Vector3 GetMaxCorner() const {
        return Vector3{ +((int)_width / 2) * _spacing, +((int)_height / 2) * _spacing, +((int)_length / 2) * _spacing };
    }

    void Draw(const Shader& shader);

protected:
    size_t _width, _height, _length;
    float _spacing;
    std::vector<Tile> _grid;
};

#endif