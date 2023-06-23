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

#include "grid.hpp"
#include "math_stuff.hpp"
#include "assets.hpp"

class MapMan;

#define TILE_SPACING_DEFAULT 2.0f

enum class Direction { Z_POS, Z_NEG, X_POS, X_NEG, Y_POS, Y_NEG };

typedef int TexID;
typedef int ModelID;

#define NO_TEX -1
#define NO_MODEL -1

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
        MatrixRotateX(ToRadians(float(-tile.pitch))), MatrixRotYDeg(float(-tile.angle)));
}

class TileGrid : public Grid<Tile>
{
public:
    //Constructs a blank TileGrid with no size
    TileGrid();
    //Constructs a TileGrid full of empty tiles.
    TileGrid(MapMan* mapMan, size_t width, size_t height, size_t length);
    //Constructs a TileGrid filled with the given tile.
    TileGrid(MapMan* mapMan, size_t width, size_t height, size_t length, float spacing, Tile fill);
    ~TileGrid();

    void SetTile(int i, int j, int k, const Tile& tile);

    //Sets a range of tiles in the grid inside of the rectangular prism with a corner at (i, j, k) and size (w, h, l).
    void SetTileRect(int i, int j, int k, int w, int h, int l, const Tile& tile);

    //Takes the tiles of `src` and places them in this grid starting at the offset at (i, j, k)
    //If the offset results in `src` exceeding the current grid's boundaries, it is cut off.
    //If `ignoreEmpty` is true, then empty tiles do not overwrite existing tiles.
    void CopyTiles(int i, int j, int k, const TileGrid &src, bool ignoreEmpty = false);

    Tile GetTile(int i, int j, int k) const;

    void UnsetTile(int i, int j, int k);

    //Returns a smaller TileGrid with a copy of the tile data in the rectangle defined by coordinates (i, j, k) and size (w, h, l).
    TileGrid Subsection(int i, int j, int k, int w, int h, int l) const;

    //Draws the tile grid, hiding all layers that are outside of the given y coordinate range.
    void Draw(Vector3 position, int fromY, int toY);
    void Draw(Vector3 position);

    //Returns a base64 encoded string with the binary representations of all tiles.
    std::string GetTileDataBase64() const;

    //Assigns tiles based on the binary data encoded in base 64. Assumes that the sizes of the data and the current grid are the same.
    void SetTileDataBase64(std::string data);

    const Model GetModel();
protected:
    MapMan* _mapMan;

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

#endif
