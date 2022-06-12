#ifndef TILE_H
#define TILE_H

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>
#include <vector>
#include <assert.h>

typedef enum Angle { ANGLE_0, ANGLE_90, ANGLE_180, ANGLE_270 } Angle;

float AngleDegrees(Angle angle);
float AngleRadians(Angle angle);
Angle AngleBack(Angle angle);
Angle AngleForward(Angle angle);

typedef struct Tile {
    Model* shape;
    Angle angle;
    Texture2D* texture;
} Tile;

bool operator==(const Tile &lhs, const Tile &rhs);
bool operator!=(const Tile &lhs, const Tile &rhs);

class TileGrid {
public:
    //Constructs a TileGrid full of empty tiles.
    TileGrid(size_t width, size_t height, size_t length, float spacing);
    //Constructs a TileGrid filled with the given tile.
    TileGrid(size_t width, size_t height, size_t length, float spacing, Tile filler);

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

    inline size_t FlatIndex(int i, int j, int k) const {
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

    //Sets a range of tiles in the grid inside of the rectangular prism with a corner at (i, j, k) and size (w, h, l).
    inline void SetTileRect(int i, int j, int k, int w, int h, int l, const Tile& tile)
    {
        assert(i >= 0 && j >= 0 && k >= 0);
        assert(i + w <= _width && j + h <= _height && k + l <= _length);
        for (int y = j; y < j + h; ++y)
        {
            for (int z = k; z < k + l; ++z)
            {
                size_t base = FlatIndex(0, y, z);
                for (int x = i; x < i + w; ++x)
                {
                    _grid[base + x] = tile;
                }
            }
        }
    }

    //Takes the tiles of `src` and places them in this grid starting at the offset at (i, j, k)
    //`src` MUST be of smaller or equal size to this grid.
    inline void CopyTiles(int i, int j, int k, const TileGrid &src)
    {
        assert(i >= 0 && j >= 0 && k >= 0);
        assert(i + src._width <= _width && j + src._height <= _height && k + src._length <= _length);
        for (int z = k; z < k + src._length; ++z) 
        {
            for (int y = j; y < j + src._height; ++y)
            {
                size_t ourBase = FlatIndex(0, y, z);
                size_t theirBase = src.FlatIndex(0, y - j, z - k);
                for (int x = i; x < i + src._width; ++x)
                {
                    _grid[ourBase + x] = src._grid[theirBase + (x - i)];
                }
            }
        }
    }

    inline Tile GetTile(int i, int j, int k) {
        return _grid[FlatIndex(i, j, k)];
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

    //Returns a smaller TileGrid with a copy of the tile data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
    TileGrid Subsection(int i, int j, int k, int w, int h, int l) const;

    void Draw();

protected:
    size_t _width, _height, _length;
    float _spacing;
    std::vector<Tile> _grid;
};

#endif