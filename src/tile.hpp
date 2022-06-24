#ifndef TILE_H
#define TILE_H

#include "raylib.h"
#include "raymath.h"
#include "json.hpp"

#include <stdlib.h>
#include <vector>
#include <assert.h>
#include <map>

#include "grid.hpp"
#include "math_stuff.hpp"
#include "assets.hpp"

#define TILE_SPACING_DEFAULT 2.0f

enum class Direction { Z_POS, Z_NEG, X_POS, X_NEG, Y_POS, Y_NEG };

struct Tile 
{
    ModelID shape;
    int angle; //In whole number of degrees
    TexID texture;
    bool flipped; //True if flipped vertically
    char padding[3]; //This is here to ensure that the byte layout is consistent across compilers.

    inline Tile() : shape(NO_MODEL), angle(0), texture(NO_TEX), flipped(false) {}
    inline Tile(ModelID s, int a, TexID t, bool f) : shape(s), angle(a), texture(t), flipped(f) {}

    inline operator bool() const
    {
        return shape != NO_MODEL && texture != NO_TEX;
    }
};

inline bool operator==(const Tile &lhs, const Tile &rhs)
{
    return (lhs.shape == rhs.shape) && (lhs.texture == rhs.texture) && (lhs.angle == rhs.angle) && (lhs.flipped == rhs.flipped);
}

inline bool operator!=(const Tile &lhs, const Tile &rhs)
{
    return !(lhs == rhs);
}

inline Matrix TileRotationMatrix(const Tile &tile)
{
    return MatrixMultiply( 
        MatrixRotateX(tile.flipped ? PI : 0.0f), MatrixRotYDeg(tile.angle));
}

class TileGrid : public Grid<Tile>
{
public:
    //Constructs a blank TileGrid with no size
    inline TileGrid()
        : TileGrid(0, 0, 0)
    {
    }
    //Constructs a TileGrid full of empty tiles.
    inline TileGrid(size_t width, size_t height, size_t length)
        : TileGrid(width, height, length, TILE_SPACING_DEFAULT, (Tile) { NO_MODEL, 0, NO_TEX, false })
    {
    }

    //Constructs a TileGrid filled with the given tile.
    inline TileGrid(size_t width, size_t height, size_t length, float spacing, Tile fill)
        : Grid<Tile>(width, height, length, spacing, fill)
    {
        _batchFromY = 0;
        _batchToY = height - 1;
        _batchPosition = Vector3Zero();
    }

    inline void SetTile(int i, int j, int k, const Tile& tile) 
    {
        SetCel(i, j, k, tile);
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
                    if (!ignoreEmpty || tile)
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
        return GetCel(i, j, k);
    }

    inline void UnsetTile(int i, int j, int k) 
    {
        _grid[FlatIndex(i, j, k)].shape = NO_MODEL;
        _regenBatches = true;
    }

    //Returns a smaller TileGrid with a copy of the tile data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
    inline TileGrid Subsection(int i, int j, int k, int w, int h, int l) const
    {
        assert(i >= 0 && j >= 0 && k >= 0);
        assert(i + w <= _width && j + h <= _height && k + l <= _length);

        TileGrid newGrid(w, h, l);

        SubsectionCopy(i, j, k, w, h, l, newGrid);

        newGrid._regenBatches = true;

        return newGrid;
    }

    //Draws the tile grid, hiding all layers that are outside of the given y coordinate range.
    void Draw(Vector3 position, int fromY, int toY);
    void Draw(Vector3 position);

    //Returns a base64 encoded string with the binary representations of all tiles.
    std::string GetTileDataBase64() const;

    //Assigns tiles based on the binary data encoded in base 64. Assumes that the sizes of the data and the current grid are the same.
    void SetTileDataBase64(std::string data);

protected:
    //Calculates lists of transformations for each tile, separated by texture and shape, to be drawn as instances.
    void _RegenBatches();

    std::map<std::pair<TexID, Mesh*>, std::vector<Matrix>> _drawBatches;
    Vector3 _batchPosition;
    bool _regenBatches;
    int _batchFromY;
    int _batchToY;
};

void to_json(nlohmann::json& j, const TileGrid &grid);
void from_json(const nlohmann::json& j, TileGrid &grid);

#endif