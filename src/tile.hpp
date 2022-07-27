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

#ifndef TILE_H
#define TILE_H

#include "raylib.h"
#include "raymath.h"
#include "json.hpp"

#include <stdlib.h>
#include <vector>
#include <assert.h>
#include <map>
#include <set>

#include "grid.hpp"
#include "math_stuff.hpp"
#include "assets.hpp"

#define TILE_SPACING_DEFAULT 2.0f

enum class Direction { Z_POS, Z_NEG, X_POS, X_NEG, Y_POS, Y_NEG };

struct Tile 
{
    ModelID shape;
    int angle; //Yaw in whole number of degrees
    TexID texture;
    int pitch; //Pitch in whole number of degrees

    // bool flipped; //True if flipped vertically
    // char padding[3]; //This is here to ensure that the byte layout is consistent across compilers.

    inline Tile() : shape(NO_MODEL), angle(0), texture(NO_TEX), pitch(0) {}
    inline Tile(ModelID s, int a, TexID t, int p) : shape(s), angle(a), texture(t), pitch(p) {}

    inline operator bool() const
    {
        return shape != NO_MODEL && texture != NO_TEX;
    }
};

inline bool operator==(const Tile &lhs, const Tile &rhs)
{
    return (lhs.shape == rhs.shape) && (lhs.texture == rhs.texture) && (lhs.angle == rhs.angle) && (lhs.pitch == rhs.pitch);
}

inline bool operator!=(const Tile &lhs, const Tile &rhs)
{
    return !(lhs == rhs);
}

inline Matrix TileRotationMatrix(const Tile &tile)
{
    return MatrixMultiply( 
        MatrixRotateX(ToRadians(tile.pitch)), MatrixRotYDeg(tile.angle));
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
        _model = nullptr;
        _regenBatches = true;
        _regenModel = true;
    }

    inline void SetTile(int i, int j, int k, const Tile& tile) 
    {
        SetCel(i, j, k, tile);
        _regenBatches = true;
        _regenModel = true;
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
        _regenModel = true;
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
        _regenModel = true;
    }

    inline Tile GetTile(int i, int j, int k) const 
    {
        return GetCel(i, j, k);
    }

    inline void UnsetTile(int i, int j, int k) 
    {
        _grid[FlatIndex(i, j, k)].shape = NO_MODEL;
        _regenBatches = true;
        _regenModel = true;
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
    //Requires lists of used textures and shapes generated by GetUsedTexturePaths() and its counterpart.
    std::string GetTileDataBase64(const std::set<fs::path> &usedTextures, const std::set<fs::path> &usedShapes) const;

    //Assigns tiles based on the binary data encoded in base 64. Assumes that the sizes of the data and the current grid are the same.
    void SetTileDataBase64(std::string data);

    std::set<fs::path> GetUsedTexturePaths() const;
    std::set<fs::path> GetUsedShapePaths() const;

    const Model &GetModel();
protected:
    //Calculates lists of transformations for each tile, separated by texture and shape, to be drawn as instances.
    void _RegenBatches(Vector3 position, int fromY, int toY);
    Model *_GenerateModel();

    std::map<std::pair<TexID, Mesh*>, std::vector<Matrix>> _drawBatches;
    Vector3 _batchPosition;
    bool _regenBatches;
    bool _regenModel;
    int _batchFromY;
    int _batchToY;

    Model *_model;
};

void to_json(nlohmann::json& j, const TileGrid &grid);
void from_json(const nlohmann::json& j, TileGrid &grid);

#endif
