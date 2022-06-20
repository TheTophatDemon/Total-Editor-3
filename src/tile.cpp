#include "tile.hpp"

#include <assert.h>

#include "assets.hpp"

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
                    TileRotationMatrix(tile), 
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