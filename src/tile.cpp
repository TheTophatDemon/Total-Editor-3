#include "tile.h"

#include <unordered_map>

TileGrid::TileGrid(size_t width, size_t height, size_t length, float spacing) {
    _width = width;
    _height = height;
    _length = length;
    _spacing = spacing;
    _grid.resize(width * height * length);
}

void TileGrid::Draw(const Shader& shader) {

    //First, a dynamic array is made for each texture found in the grid. These arrays contain the indexes of each tile with that texture.
    std::unordered_map<Texture2D*, std::vector<size_t>> textureGroups;
    for (size_t i = 0; i < _width; ++i) {
        for (size_t j = 0; j < _height; ++j) {
            for (size_t k = 0; k < _length; ++k) {
                size_t t = FlatIndex(i, j, k);
                if (_grid[t].texture != nullptr && _grid[t].shape != nullptr) {
                    //Instantiate the vector if it hasn't been made already
                    if (textureGroups.find(_grid[t].texture) == textureGroups.end()) {
                        textureGroups[_grid[t].texture] = std::vector<size_t>();
                    }
                    textureGroups[_grid[t].texture].push_back(t);
                }
            }
        }
    }

    for (auto& [tex, tileIndexes] : textureGroups) {
        //For each group of tiles with a common texture, generate a material for that texture.
        Material material = LoadMaterialDefault();
        material.shader = shader;
        SetMaterialTexture(&material, MATERIAL_MAP_ALBEDO, *tex);

        //Then group all of the tiles with this texture by their meshes. This time, the array records each tile's world space transform.
        std::unordered_map<Mesh*, std::vector<Matrix>> shapeGroups;
        for (const size_t t : tileIndexes) {
            Tile& tile = _grid[t];
            Vector3 worldPos = GridToWorldPos(UnflattenIndex(t), true);
            Matrix matrix = MatrixMultiply(
                MatrixRotateY(AngleRadians(tile.angle)), 
                MatrixTranslate(worldPos.x, worldPos.y, worldPos.z));

            for (int m = 0; m < tile.shape->meshCount; ++m) {
                if (shapeGroups.find(&tile.shape->meshes[m]) == shapeGroups.end()) {
                    shapeGroups[&tile.shape->meshes[m]] = std::vector<Matrix>();
                }
                shapeGroups[&tile.shape->meshes[m]].push_back(matrix);
            }
        }

        //This allows us to call DrawMeshInstanced for each combination of material and mesh.
        for (auto& [mesh, matrices] : shapeGroups) {
            DrawMeshInstanced(*mesh, material, matrices.data(), matrices.size());
        }
    }
}