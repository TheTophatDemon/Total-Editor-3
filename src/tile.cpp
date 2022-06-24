#include "tile.hpp"

#include "cppcodec/base64_default_rfc4648.hpp"

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
            if (tile) {
                //Calculate world space matrix for the tile
                Vector3 gridPos = UnflattenIndex(t);
                Vector3 worldPos = Vector3Add(_batchPosition, GridToWorldPos(gridPos, true));
                Matrix matrix = MatrixMultiply(
                    TileRotationMatrix(tile), 
                    MatrixTranslate(worldPos.x, worldPos.y, worldPos.z));

                const Model &shape = Assets::ModelFromID(tile.shape);
                for (size_t m = 0; m < shape.meshCount; ++m) {
                    //Add the tile's transform to the instance arrays for each mesh
                    auto pair = std::make_pair(tile.texture, &shape.meshes[m]);
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
        DrawMeshInstanced(*pair.second, Assets::GetMaterialForTexture(pair.first, true), matrices.data(), matrices.size());
    }
}

std::string TileGrid::GetTileDataBase64() const 
{
    std::vector<uint8_t> bin;
    bin.reserve(_grid.size() * sizeof(Tile));
    for (size_t i = 0; i < _grid.size(); ++i)
    {
        //Reinterpret each tile as a series of bytes and push them onto the vector.
        const char *tileBin = reinterpret_cast<const char *>(&_grid[i]);
        for (size_t b = 0; b < sizeof(Tile); ++b)
        {
            bin.push_back(tileBin[b]);
        }
    }

    return base64::encode(bin);
}

void TileGrid::SetTileDataBase64(std::string data)
{
    std::vector<uint8_t> bin = base64::decode(data);
    for (size_t i = 0; i < bin.size() / sizeof(Tile) && i < _grid.size(); ++i)
    {
        //Reinterpret groups of bytes as tiles and place them into the grid.
        _grid[i] = *(reinterpret_cast<Tile *>(&bin[i * sizeof(Tile)]));
    }
}

void to_json(nlohmann::json& j, const TileGrid &grid)
{
    j["width"] = grid.GetWidth();
    j["height"] = grid.GetHeight();
    j["length"] = grid.GetLength();
    j["data"] = grid.GetTileDataBase64();
}

void from_json(const nlohmann::json& j, TileGrid &grid)
{
    grid = TileGrid(j.at("width"), j.at("height"), j.at("length"), TILE_SPACING_DEFAULT, Tile());
    grid.SetTileDataBase64(j.at("data"));
}