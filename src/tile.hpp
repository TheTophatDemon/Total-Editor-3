/**
 * Copyright (c) 2022-present Alexander Lunsford
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
#include <memory>
#include <functional>
#include <array>

#include "grid.hpp"
#include "math_stuff.hpp"
#include "assets.hpp"

class MapMan;

#define TILE_SPACING_DEFAULT 2.0f

enum class Direction { Z_POS, Z_NEG, X_POS, X_NEG, Y_POS, Y_NEG };

typedef int16_t TexID;
typedef int16_t ModelID;

#define NO_TEX (int16_t)(-1)
#define NO_MODEL (int16_t)(-1)
#define TEXTURES_PER_TILE 2

struct Tile 
{
    ModelID shape;
    std::array<TexID, TEXTURES_PER_TILE> textures;
    uint8_t yaw, pitch; // These are values in the range of 0-3 representing 90 degree rotations.

    inline Tile() : shape(NO_MODEL), yaw(0), pitch(0) {}
    
    inline Tile(ModelID shape, TexID tex1, TexID tex2, uint8_t yaw, uint8_t pitch)
        : shape(shape), yaw(yaw), pitch(pitch) 
    {
        textures[0] = tex1;
        textures[1] = tex2;
    }

    inline operator bool() const
    {
        return shape != NO_MODEL;
    }
};

inline bool operator==(const Tile &lhs, const Tile &rhs)
{
    if (lhs.shape != rhs.shape) return false;
    for (int i = 0; i < TEXTURES_PER_TILE; ++i) 
    {
        if (lhs.textures[i] != rhs.textures[i]) return false;
    }
    return (lhs.yaw == rhs.yaw) && (lhs.pitch == rhs.pitch);
}

inline bool operator!=(const Tile &lhs, const Tile &rhs)
{
    return !(lhs == rhs);
}

inline Matrix TileRotationMatrix(uint8_t tileYaw, uint8_t tilePitch)
{
    return MatrixRotateX(float(tilePitch % 4) * -PI / 2.0f) * MatrixRotateY(float(tileYaw % 4) * -PI / 2.0f);
}

class TileGrid : public Grid<Tile>
{
public:
    // Constructs a TileGrid full of empty tiles.
    TileGrid(MapMan& mapMan, size_t width, size_t height, size_t length);
    // Constructs a TileGrid filled with the given tile.
    TileGrid(MapMan& mapMan, size_t width, size_t height, size_t length, float spacing, Tile fill);

    TileGrid(const TileGrid& other) = default;
    TileGrid& operator=(const TileGrid& other) = default;

    Tile GetTile(int i, int j, int k) const;
    Tile GetTile(int flatIndex) const;
    void SetTile(int i, int j, int k, const Tile& tile);
    void SetTile(int flatIndex, const Tile& tile);

    // Sets a range of tiles in the grid inside of the rectangular prism with a corner at (i, j, k) and size (w, h, l).
    void SetTileRect(int i, int j, int k, int w, int h, int l, const Tile& tile);

    // Takes the tiles of `src` and places them in this grid starting at the offset at (i, j, k)
    // If the offset results in `src` exceeding the current grid's boundaries, it is cut off.
    // If `ignoreEmpty` is true, then empty tiles do not overwrite existing tiles.
    void CopyTiles(int i, int j, int k, const TileGrid &src, bool ignoreEmpty = false);


    void UnsetTile(int i, int j, int k);

    // Returns a smaller TileGrid with a copy of the tile data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
    TileGrid Subsection(int i, int j, int k, int w, int h, int l) const;

    // Draws the tile grid, hiding all layers that are outside of the given y coordinate range.
    void Draw(Vector3 position, int fromY, int toY);
    void Draw(Vector3 position);


    // Returns a base64 encoded string with the binary representations of all tiles.
    std::string GetTileDataBase64() const;

    // Assigns tiles based on base 64 encoded data from Total Edtor 3.1 or earlier.
    void SetTileDataBase64OldFormat(std::string data);

    // Assigns tiles based on the binary data encoded in base 64. Assumes that the sizes of the data and the current grid are the same.
    void SetTileDataBase64(std::string data);

    // Returns the list of texture and model IDs that are actually used in this tile grid
    std::pair<std::vector<TexID>, std::vector<ModelID>> GetUsedIDs() const;

    const Model GetModel();
protected:
    std::reference_wrapper<MapMan> _mapMan;

    // Calculates lists of transformations for each tile, separated by texture and shape, to be drawn as instances.
    void _RegenBatches(Vector3 position, int fromY, int toY);
    // Combines all of the tiles into a single model, for export or for preview. When culling is true, redundant faces between tiles are removed.
    Model* _GenerateModel(bool culling = true);

    std::map<std::pair<TexID, Mesh*>, std::vector<Matrix>> _drawBatches;
    
    Vector3 _batchPosition;
    bool _regenBatches;
    bool _regenModel;
    int _batchFromY;
    int _batchToY;

    Model *_model;
    bool _modelCulled;
};

#endif
