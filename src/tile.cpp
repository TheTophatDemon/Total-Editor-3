/*
Copyright (C) 2022 Alexander Lunsford
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "tile.hpp"

#include "cppcodec/base64_default_rfc4648.hpp"

#include <assert.h>
#include <iostream>
#include <algorithm>

#include "assets.hpp"
#include "app.hpp"

void TileGrid::_RegenBatches(Vector3 position, int fromY, int toY)
{
    _drawBatches.clear();
    _batchFromY = fromY;
    _batchToY = toY;
    _batchPosition = position;
    _regenBatches = false;

    const size_t layerArea = _width * _length;
    //Create a hash map of dynamic arrays for each combination of texture and mesh
    for (int y = fromY; y <= toY; ++y)
    {
        for (size_t t = y * layerArea; t < (y + 1) * layerArea; ++t) {
            const Tile& tile = _grid[t];
            if (tile) {
                //Calculate world space matrix for the tile
                Vector3 gridPos = UnflattenIndex(t);
                Vector3 worldPos = Vector3Add(position, GridToWorldPos(gridPos, true));
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
    if (App::Get()->IsPreviewing())
    {
        DrawModel(GetModel(), position, 1.0f, WHITE);
    }
    else
    {
        if (_regenBatches || fromY != _batchFromY || toY != _batchToY || !Vector3Equals(position, _batchPosition))
        {
            _RegenBatches(position, fromY, toY);
        }

        //Call DrawMeshInstanced for each combination of material and mesh.
        for (auto& [pair, matrices] : _drawBatches) {
            DrawMeshInstanced(*pair.second, Assets::GetMaterialForTexture(pair.first, true), matrices.data(), matrices.size());
        }
    }
}

std::string TileGrid::GetTileDataBase64(const std::set<fs::path> &usedTextures, const std::set<fs::path> &usedShapes) const 
{
    std::vector<fs::path> textures;
    std::vector<fs::path> shapes;

    for (const fs::path &p : usedTextures) textures.push_back(p);
    for (const fs::path &p : usedShapes) shapes.push_back(p);

    std::vector<uint8_t> bin;
    bin.reserve(_grid.size() * sizeof(Tile));
    for (size_t i = 0; i < _grid.size(); ++i)
    {
        Tile savedTile = _grid[i];

        if (savedTile)
        {
            //Change the texture and shape IDs to index into the given two arrays.
            fs::path tPath = Assets::PathFromTexID(savedTile.texture);
            fs::path sPath = Assets::PathFromModelID(savedTile.shape);
            for (int i = 0; i < textures.size(); ++i)
            {
                if (textures[i] == tPath) savedTile.texture = i;
            }
            for (int i = 0; i < shapes.size(); ++i)
            {
                if (shapes[i] == sPath) savedTile.shape = i;
            }
        }

        //Reinterpret each tile as a series of bytes and push them onto the vector.
        const char *tileBin = reinterpret_cast<const char *>(&savedTile);
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
    std::cout << std::endl;
}

#define MAX_MATERIAL_MAPS 12
Model *TileGrid::_GenerateModel()
{
    _RegenBatches(Vector3Zero(), 0, _height - 1);

    std::set<TexID> usedTexIDs;
    struct DynMesh {
        std::vector<float> positions;
        std::vector<float> texCoords;
        std::vector<float> normals;
        std::vector<unsigned char> colors;
        std::vector<unsigned short> indices;
        int triCount; //Independent form indices count since some models may not have indices
    };
    std::map<TexID, DynMesh> meshMap;

    for (const auto& [pair, matrices] : _drawBatches)
    {
        auto [iter, succ] = usedTexIDs.insert(pair.first);
        if (succ)
        {
            meshMap[pair.first] = (DynMesh) {};
            meshMap[pair.first].triCount = 0;
        }
        Mesh &shape = *pair.second;
        DynMesh &mesh = meshMap[pair.first];
        //Generate vertex data for this tile.
        for (const Matrix &matrix : matrices)
        {
            int vBase = mesh.positions.size();
            mesh.triCount += shape.triangleCount;
            for (int v = 0; v < shape.vertexCount; v++)
            {
                if (shape.vertices != NULL)
                {
                    //Transform shape vertices into tile's orientation and position
                    Vector3 vec = (Vector3) { shape.vertices[v*3], shape.vertices[v*3 + 1], shape.vertices[v*3 + 2] };
                    vec = Vector3Transform(vec, matrix);
                    mesh.positions.push_back(vec.x);
                    mesh.positions.push_back(vec.y);
                    mesh.positions.push_back(vec.z);
                }

                if (shape.normals != NULL)
                {
                    //Transform normals by the tile's rotation, but not its position
                    Matrix rotMatrix = matrix;
                    rotMatrix.m12 = 0.0f;
                    rotMatrix.m13 = 0.0f;
                    rotMatrix.m14 = 0.0f;
                    
                    Vector3 norm = (Vector3) { shape.normals[v*3], shape.normals[v*3 + 1], shape.normals[v*3 + 2] };
                    norm = Vector3Transform(norm, rotMatrix);
                    mesh.normals.push_back(norm.x);
                    mesh.normals.push_back(norm.y);
                    mesh.normals.push_back(norm.z);
                }

                if (shape.texcoords != NULL)
                {
                    //Tex coordinates are just copied into the aggregate mesh
                    mesh.texCoords.push_back(shape.texcoords[v*2]);
                    mesh.texCoords.push_back(shape.texcoords[v*2 + 1]);
                }

                if (shape.colors != NULL)
                {
                    //Vertex colors are not supported in shapes, so just fill it with white.
                    mesh.colors.push_back(shape.colors[v*4 + 0]);
                    mesh.colors.push_back(shape.colors[v*4 + 1]);
                    mesh.colors.push_back(shape.colors[v*4 + 2]);
                    mesh.colors.push_back(shape.colors[v*4 + 3]);
                }
            }
            if (shape.indices != NULL)
            {
                for (int i = 0; i < shape.triangleCount * 3; i++)
                {
                    //Add indicies, but with the offset of the current tile's vertices.
                    mesh.indices.push_back((unsigned short)(vBase + shape.indices[i]));
                }
            }
        }
    }

    //Create Raylib mesh
    Model *model = (Model *)RL_MALLOC(sizeof(Model));
    model->materialCount = usedTexIDs.size();
    model->meshCount = usedTexIDs.size();
    model->meshMaterial = (int *)RL_CALLOC(usedTexIDs.size(), sizeof(int));
    model->materials = (Material *)RL_CALLOC(usedTexIDs.size(), sizeof(Material));
    model->meshes = (Mesh *)RL_CALLOC(usedTexIDs.size(), sizeof(Mesh));
    model->transform = MatrixIdentity();
    model->bindPose = NULL;
    model->boneCount = 0;
    model->bones = NULL;
    
    auto texID = usedTexIDs.cbegin();
    for (int i = 0; i < model->materialCount; ++i)
    {
        model->materials[i] = Assets::GetMaterialForTexture(*texID, false);
        model->meshMaterial[i] = i;

        //Copy mesh data into Raylib mesh
        DynMesh &dMesh = meshMap[*texID];
        model->meshes[i] = (Mesh) { 0 };
        model->meshes[i].vertexCount = dMesh.positions.size() / 3;
        model->meshes[i].triangleCount = dMesh.triCount;
        
        if (dMesh.positions.size() > 0)
        {
            model->meshes[i].vertices = (float *) RL_CALLOC(dMesh.positions.size(), sizeof(float));
            memcpy(model->meshes[i].vertices, dMesh.positions.data(), dMesh.positions.size() * sizeof(float));
        }
        if (dMesh.texCoords.size() > 0)
        {
            model->meshes[i].texcoords = (float *) RL_CALLOC(dMesh.texCoords.size(), sizeof(float));
            memcpy(model->meshes[i].texcoords, dMesh.texCoords.data(), dMesh.texCoords.size() * sizeof(float));
        }
        if (dMesh.normals.size() > 0)
        {
            model->meshes[i].normals = (float *) RL_CALLOC(dMesh.normals.size(), sizeof(float));
            memcpy(model->meshes[i].normals, dMesh.normals.data(), dMesh.normals.size() * sizeof(float));
        }
        if (dMesh.colors.size() > 0)
        {
            model->meshes[i].colors = (unsigned char *) RL_CALLOC(dMesh.colors.size(), sizeof(float));
            memcpy(model->meshes[i].colors, dMesh.colors.data(), dMesh.colors.size() * sizeof(float));
        }
        if (dMesh.indices.size() > 0)
        {
            model->meshes[i].indices = (unsigned short *) RL_CALLOC(dMesh.indices.size(), sizeof(unsigned char));
            memcpy(model->meshes[i].indices, dMesh.indices.data(), dMesh.indices.size() * sizeof(float));
        }

        UploadMesh(&model->meshes[i], false);

        texID++;
    }

    return model;
}

const Model &TileGrid::GetModel()
{
    if (_regenModel || _model == nullptr)
    {
        if (_model != nullptr)
        {
            //Cannot use UnloadModel() or it will unload the materials that are used elsewhere.
            RL_FREE(_model->materials);
            RL_FREE(_model->meshMaterial);
            for (int m = 0; m < _model->meshCount; ++m)
            {
                if (_model->meshes[m].vertices != NULL) RL_FREE(_model->meshes[m].vertices);
                if (_model->meshes[m].texcoords != NULL) RL_FREE(_model->meshes[m].texcoords);
                if (_model->meshes[m].normals != NULL) RL_FREE(_model->meshes[m].normals);
                if (_model->meshes[m].colors != NULL) RL_FREE(_model->meshes[m].colors);
                if (_model->meshes[m].indices != NULL) RL_FREE(_model->meshes[m].indices);
                if (_model->meshes[m].vboId != NULL) RL_FREE(_model->meshes[m].vboId);
            }
            RL_FREE(_model->meshes);
            free(_model);
        }
        _model = _GenerateModel();
        _regenModel = false;
    }

    return *_model;
}

void to_json(nlohmann::json& j, const TileGrid &grid)
{
    j["width"] = grid.GetWidth();
    j["height"] = grid.GetHeight();
    j["length"] = grid.GetLength();
    std::set<fs::path> usedTextures = grid.GetUsedTexturePaths();
    std::set<fs::path> usedShapes = grid.GetUsedShapePaths();
    j["textures"] = usedTextures;
    j["shapes"] = usedShapes;
    j["data"] = grid.GetTileDataBase64(usedTextures, usedShapes);
}

void from_json(const nlohmann::json& j, TileGrid &grid)
{
    grid = TileGrid(j.at("width"), j.at("height"), j.at("length"), TILE_SPACING_DEFAULT, Tile());
    grid.SetTileDataBase64(j.at("data"));
}

std::set<fs::path> TileGrid::GetUsedTexturePaths() const
{
    std::set<fs::path> paths;
    for (const Tile &tile : _grid)
    {
        paths.insert(Assets::PathFromTexID(tile.texture));
    }
    return paths;
}

std::set<fs::path> TileGrid::GetUsedShapePaths() const
{
    std::set<fs::path> paths;
    for (const Tile &tile : _grid)
    {
        paths.insert(Assets::PathFromModelID(tile.shape));
    }
    return paths;
}