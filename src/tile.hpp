#ifndef TILE_H
#define TILE_H

#include "raylib.h"
#include "raymath.h"

#include <stdlib.h>
#include <vector>
#include <assert.h>
#include <map>

#include "math_stuff.hpp"

typedef enum Angle { ANGLE_0, ANGLE_90, ANGLE_180, ANGLE_270, ANGLE_COUNT } Angle;

float AngleDegrees(Angle angle);
float AngleRadians(Angle angle);
Angle AngleBack(Angle angle);
Angle AngleForward(Angle angle);
Matrix AngleMatrix(Angle angle);

typedef struct Tile {
    Model* shape;
    Angle angle;
    Texture2D* texture;
} Tile;

bool operator==(const Tile &lhs, const Tile &rhs);
bool operator!=(const Tile &lhs, const Tile &rhs);

class TileGrid 
{
public:
    //Constructs a blank TileGrid with no size
    TileGrid();
    //Constructs a TileGrid full of empty tiles.
    TileGrid(size_t width, size_t height, size_t length, float spacing);
    //Constructs a TileGrid filled with the given tile.
    TileGrid(size_t width, size_t height, size_t length, float spacing, Tile filler);

    inline Vector3 WorldToGridPos(Vector3 worldPos) const 
    {
        return Vector3{ floorf(worldPos.x / _spacing), floorf(worldPos.y / _spacing) , floorf(worldPos.z / _spacing)};
    }

    //Converts (whole number) grid cel coordinates to world coordinates.
    //If `center` is true, then the world coordinate will be in the center of the cel instead of the corner.
    inline Vector3 GridToWorldPos(Vector3 gridPos, bool center) const 
    {
        if (center) 
        {
            return (Vector3) {
                (gridPos.x * _spacing) + (_spacing / 2.0f),
                (gridPos.y * _spacing) + (_spacing / 2.0f),
                (gridPos.z * _spacing) + (_spacing / 2.0f),
            };
        } 
        else 
        {
            return Vector3{ gridPos.x * _spacing, gridPos.y * _spacing, gridPos.z * _spacing };
        }
    }

    inline Vector3 SnapToCelCenter(Vector3 worldPos) const 
    {
        worldPos.x = (floorf(worldPos.x / _spacing) * _spacing) + (_spacing / 2.0f);
        worldPos.y = (floorf(worldPos.y / _spacing) * _spacing) + (_spacing / 2.0f);
        worldPos.z = (floorf(worldPos.z / _spacing) * _spacing) + (_spacing / 2.0f);
        return worldPos;
    }

    inline size_t FlatIndex(int i, int j, int k) const 
    {
        return i + (k * _width) + (j * _width * _length);
    }

    inline Vector3 UnflattenIndex(size_t idx) const 
    {
        return Vector3{
            (float)(idx % _width),
            (float)(idx / (_width * _length)),
            (float)((idx / _width) % _length)
        };
    }

    inline void SetTile(int i, int j, int k, const Tile& tile) 
    {
        _grid[FlatIndex(i, j, k)] = tile;
        _regenBatches = true;
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
        _regenBatches = true;
    }

    //Takes the tiles of `src` and places them in this grid starting at the offset at (i, j, k)
    //If the offset results in `src` exceeding the current grid's boundaries, it is cut off.
    //If `ignoreEmpty` is true, then empty tiles do not overwrite existing tiles.
    inline void CopyTiles(int i, int j, int k, const TileGrid &src, bool ignoreEmpty = false)
    {
        assert(i >= 0 && j >= 0 && k >= 0);
        int xEnd = Min(i + src._width, _width); 
        int yEnd = Min(j + src._height, _height);
        int zEnd = Min(k + src._length, _length);
        for (int z = k; z < zEnd; ++z) 
        {
            for (int y = j; y < yEnd; ++y)
            {
                size_t ourBase = FlatIndex(0, y, z);
                size_t theirBase = src.FlatIndex(0, y - j, z - k);
                for (int x = i; x < xEnd; ++x)
                {
                    const Tile &tile = src._grid[theirBase + (x - i)];
                    if (!ignoreEmpty || tile.shape != nullptr)
                    {
                        _grid[ourBase + x] = tile;
                    }
                }
            }
        }
        _regenBatches = true;
    }

    inline Tile GetTile(int i, int j, int k) const 
    {
        return _grid[FlatIndex(i, j, k)];
    }

    inline void UnsetTile(int i, int j, int k) 
    {
        _grid[FlatIndex(i, j, k)].shape = nullptr;
        _regenBatches = true;
    }

    inline size_t GetWidth() const { return _width; }
    inline size_t GetHeight() const { return _height; }
    inline size_t GetLength() const { return _length; }
    inline float GetSpacing() const { return _spacing; }

    inline Vector3 GetMinCorner() const 
    {
        return Vector3Zero();
    }
    
    inline Vector3 GetMaxCorner() const 
    {
        return (Vector3) { (float)_width * _spacing, (float)_height * _spacing, (float)_length * _spacing };
    }

    inline Vector3 GetCenterPos() const 
    {
        return (Vector3) { (float)_width * _spacing / 2.0f, (float)_height * _spacing / 2.0f, (float)_length * _spacing / 2.0f };
    }

    //Returns a smaller TileGrid with a copy of the tile data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
    TileGrid Subsection(int i, int j, int k, int w, int h, int l) const;

    //Draws the tile grid, hiding all layers that are outside of the given y coordinate range.
    void Draw(Vector3 position, int fromY, int toY);
    void Draw(Vector3 position);

protected:
    //Calculates lists of transformations for each tile, separated by texture and shape, to be drawn as instances.
    void _RegenBatches();

    size_t _width, _height, _length;
    float _spacing;
    std::vector<Tile> _grid;
    std::map<std::pair<Texture2D*, Mesh*>, std::vector<Matrix>> _drawBatches;
    bool _regenBatches;
    int _batchFromY;
    int _batchToY;
    Vector3 _batchPosition;
};

#endif