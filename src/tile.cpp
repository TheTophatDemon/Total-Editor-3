#include "tile.hpp"

#include <assert.h>

#include "assets.hpp"

#define TILE_SPACING_DEFAULT 2.0f

float AngleDegrees(Angle angle) 
{
    return (float)(((int)angle) * 90);
}

float AngleRadians(Angle angle) 
{
    return (float)angle * PI / 2.0f;
}

Angle AngleBack(Angle angle) 
{
    return (Angle)((angle - 1) < ANGLE_0 ? ANGLE_270 : (angle - 1));
}

Angle AngleForward(Angle angle) 
{
    return (Angle)((angle + 1) > ANGLE_270 ? ANGLE_0 : (angle + 1));
}

static const Matrix ANGLE_MATRICES[ANGLE_COUNT] = {
    MatrixRotateY(0.0f), MatrixRotateY(PI / 2.0f), MatrixRotateY(PI), MatrixRotateY(3.0f * PI / 2.0f)
};

Matrix AngleMatrix(Angle angle)
{
    return ANGLE_MATRICES[angle];
}

bool operator==(const Tile &lhs, const Tile &rhs)
{
    return (lhs.shape == rhs.shape) && (lhs.texture == rhs.texture) && (lhs.angle == rhs.angle);
}

bool operator!=(const Tile &lhs, const Tile &rhs)
{
    return !(lhs == rhs);
}

//Create an empty, zero-size tile grid.
TileGrid::TileGrid() : TileGrid(0, 0, 0, TILE_SPACING_DEFAULT)
{
}

//Creates a tile grid that is empty
TileGrid::TileGrid(size_t width, size_t height, size_t length, float spacing) : TileGrid(width, height, length, spacing, (Tile){ nullptr, ANGLE_0, nullptr }) 
{
}

//Create a tile grid filled with the given tile.
TileGrid::TileGrid(size_t width, size_t height, size_t length, float spacing, Tile filler) 
{
    _width = width;
    _height = height;
    _length = length;
    _spacing = spacing;
    _grid.resize(width * height * length);

    for (size_t i = 0; i < _grid.size(); ++i)
    {
        _grid[i] = filler;
    }

    _regenBatches = true;
    _batchFromY = 0;
    _batchToY = height - 1;
    _batchPosition = Vector3Zero();
}

void TileGrid::_RegenBatches()
{
    _drawBatches.clear();

    const size_t layerArea = _width * _length;
    //Create a hash map of dynamic arrays for each combination of texture and mesh
    for (int y = _batchFromY; y <= _batchToY; ++y)
    {
        for (size_t t = y * layerArea; t < (y + 1) * layerArea; ++t) {
            const Tile& tile = _grid[t];
            if (tile.texture && tile.shape) {
                //Calculate world space matrix for the tile
                Vector3 gridPos = UnflattenIndex(t);
                Vector3 worldPos = Vector3Add(_batchPosition, GridToWorldPos(gridPos, true));
                Matrix matrix = MatrixMultiply(
                    AngleMatrix(tile.angle), 
                    MatrixTranslate(worldPos.x, worldPos.y, worldPos.z));

                for (size_t m = 0; m < tile.shape->meshCount; ++m) {
                    //Add the tile's transform to the instance arrays for each mesh
                    auto pair = std::make_pair(tile.texture, &tile.shape->meshes[m]);
                    if (_drawBatches.find(pair) == _drawBatches.end()) {
                        //Put in a vector for this pair if there hasn't been one already
                        _drawBatches[pair] = std::vector<Matrix>();
                    }
                    _drawBatches[pair].push_back(matrix);
                }
            }
        }
    }
}

void TileGrid::Draw(Vector3 position)
{
    Draw(position, 0, _height - 1);
}

void TileGrid::Draw(Vector3 position, int fromY, int toY)
{
    if (_regenBatches || fromY != _batchFromY || toY != _batchToY || !Vector3Equals(position, _batchPosition))
    {
        _batchFromY = fromY;
        _batchToY = toY;
        _batchPosition = position;
        _RegenBatches();
        _regenBatches = false;
    }

    //Call DrawMeshInstanced for each combination of material and mesh.
    for (auto& [pair, matrices] : _drawBatches) {
        DrawMeshInstanced(*pair.second, *Assets::GetMaterialForTexture(pair.first, true), matrices.data(), matrices.size());
    }
}

TileGrid TileGrid::Subsection(int i, int j, int k, int w, int h, int l) const
{
    assert(i >= 0 && j >= 0 && k >= 0);
    assert(i + w <= _width && j + h <= _height && k + l <= _length);

    TileGrid newGrid(w, h, l, _spacing);

    for (int z = k; z < k + l; ++z) 
    {
        for (int y = j; y < j + h; ++y)
        {
            size_t ourBase = FlatIndex(0, y, z);
            size_t theirBase = newGrid.FlatIndex(0, y - j, z - k);
            for (int x = i; x < i + w; ++x)
            {
                newGrid._grid[theirBase + (x - i)] = _grid[ourBase + x];
            }
        }
    }

    newGrid._regenBatches = true;

    return newGrid;
}