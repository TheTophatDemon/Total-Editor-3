#include "tile.hpp"

#include <map>
#include <assert.h>

float AngleDegrees(Angle angle) {
    return (float)(((int)angle) * 90);
}

float AngleRadians(Angle angle) {
    return (float)angle * PI / 2.0f;
}

Angle AngleBack(Angle angle) {
    return (Angle)((angle - 1) < ANGLE_0 ? ANGLE_270 : (angle - 1));
}

Angle AngleForward(Angle angle) {
    return (Angle)((angle + 1) > ANGLE_270 ? ANGLE_0 : (angle + 1));
}

TileGrid::TileGrid(size_t width, size_t height, size_t length, float spacing) {
    _width = width;
    _height = height;
    _length = length;
    _spacing = spacing;
    _grid.resize(width * height * length);
    
    //Make sure the grid is full of empty tiles
    for (Tile& tile : _grid) {
        tile.shape = nullptr;
        tile.material = nullptr;
    }
}

TileGrid::TileGrid(size_t width, size_t height, size_t length, float spacing, Tile filler) {
    _width = width;
    _height = height;
    _length = length;
    _spacing = spacing;
    _grid.resize(width * height * length);
    
    for (size_t i = 0; i < _grid.size(); ++i)
    {
        _grid[i] = filler;
    }
}

void TileGrid::Draw() {
    //Create a hash map of dynamic arrays for each combination of material and mesh
    auto groups = std::map<std::pair<Material*, Mesh*>, std::vector<Matrix>>();
    for (size_t t = 0; t < _grid.size(); ++t) {
        const Tile& tile = _grid[t];
        if (tile.material && tile.shape) {
            //Calculate world space matrix for the tile
            Vector3 worldPos = GridToWorldPos(UnflattenIndex(t), true);
            Matrix matrix = MatrixMultiply(
                MatrixRotateY(AngleRadians(tile.angle)), 
                MatrixTranslate(worldPos.x, worldPos.y, worldPos.z));

            for (size_t m = 0; m < tile.shape->meshCount; ++m) {
                //Add the tile's transform to the instance arrays for each mesh
                auto pair = std::make_pair(tile.material, &tile.shape->meshes[m]);
                if (groups.find(pair) == groups.end()) {
                    //Put in a vector for this pair if there hasn't been one already
                    groups[pair] = std::vector<Matrix>();
                }
                groups[pair].push_back(matrix);
            }
        }
    }

    //This allows us to call DrawMeshInstanced for each combination of material and mesh.
    for (auto& [pair, matrices] : groups) {
        DrawMeshInstanced(*pair.second, *pair.first, matrices.data(), matrices.size());
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

    return newGrid;
}