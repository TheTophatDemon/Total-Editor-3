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

#include "map_man.hpp"

#include "rlgl.h"
#include "json.hpp"
#include "cppcodec/base64_default_rfc4648.hpp"

#include <fstream>
#include <iostream>
#include <limits>

#include "app.hpp"
#include "assets.hpp"

void MapMan::_Execute(std::shared_ptr<Action> action)
{
    _undoHistory.push_back(action);
    if (_undoHistory.size() > App::Get()->GetUndoMax()) _undoHistory.pop_front();
    _redoHistory.clear();
    _undoHistory.back()->Do(*this);
}

void MapMan::ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, Tile newTile)
{
    TileGrid prevState = _tileGrid.Subsection(i, j, k, w, h, l);
    TileGrid newState = TileGrid(this, w, h, l, _tileGrid.GetSpacing(), newTile);

    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<TileAction>(i, j, k, prevState, newState)
    ));
}

void MapMan::ExecuteTileAction(size_t i, size_t j, size_t k, size_t w, size_t h, size_t l, TileGrid brush)
{
    const TileGrid prevState = _tileGrid.Subsection(
            i, j, k, 
            Min(w, _tileGrid.GetWidth() - i),  //Cut off parts that go beyond map boundaries
            Min(h, _tileGrid.GetHeight() - j), 
            Min(l, _tileGrid.GetLength() - k)
        );

    TileGrid newState = prevState; //Copy the old state and merge the brush into it
    newState.CopyTiles(0, 0, 0, brush, true);

    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<TileAction>(i, j, k, prevState, newState)
    ));
}

void MapMan::ExecuteEntPlacement(int i, int j, int k, Ent newEnt)
{
    Ent prevEnt = _entGrid.HasEnt(i, j, k) ? _entGrid.GetEnt(i, j, k) : Ent{ 0 };
    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<EntAction>(i, j, k, _entGrid.HasEnt(i, j, k), false, prevEnt, newEnt)
    ));
}

void MapMan::ExecuteEntRemoval(int i, int j, int k)
{
    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<EntAction>(i, j, k, true, true, _entGrid.GetEnt(i, j, k), Ent{ 0 })
    ));
}

bool MapMan::SaveTE3Map(fs::path filePath)
{
    using namespace nlohmann;

    try
    {
        json jData;
        jData["tiles"] = json::object();
        jData["tiles"]["width"] = _tileGrid.GetWidth();
        jData["tiles"]["height"] = _tileGrid.GetHeight();
        jData["tiles"]["length"] = _tileGrid.GetLength();

        //Make new texture & model lists containing only used assets
        //This prevents extraneous assets from accumulating in the file every time it's saved
        auto [usedTexIDs, usedModelIDs] = _tileGrid.GetUsedIDs();
        std::vector<std::string> usedTexPaths, usedModelPaths;
        usedTexPaths.resize(usedTexIDs.size());
        usedModelPaths.resize(usedModelIDs.size());
        std::transform(usedTexIDs.begin(), usedTexIDs.end(), usedTexPaths.begin(), 
            [&](TexID id){
                return _textureList[id]->GetPath().generic_string();
            });
        std::transform(usedModelIDs.begin(), usedModelIDs.end(), usedModelPaths.begin(),
            [&](ModelID id){
                return _modelList[id]->GetPath().generic_string();
            });

        jData["tiles"]["textures"] = usedTexPaths;
        jData["tiles"]["shapes"] = usedModelPaths;

        //Make a copy of the map that reassigns all IDs to match the new lists
        const int tileArea = _tileGrid.GetWidth() * _tileGrid.GetHeight() * _tileGrid.GetLength();
        TileGrid optimizedGrid = _tileGrid.Subsection(0, 0, 0, _tileGrid.GetWidth(), _tileGrid.GetHeight(), _tileGrid.GetLength());
        for (size_t i = 0; i < tileArea; ++i)
        {
            Tile tile = optimizedGrid.GetTile(i);
            for (int t = 0; t < usedTexIDs.size(); ++t)
            {
                if (usedTexIDs[t] == tile.texture)
                {
                    tile.texture = t;
                    break;
                }
            }
            for (int m = 0; m < usedModelIDs.size(); ++m)
            {
                if (usedModelIDs[m] == tile.shape)
                {
                    tile.shape = m;
                    break;
                }
            }
            optimizedGrid.SetTile(i, tile);
        }

        //Save the modified tile data
        jData["tiles"]["data"] = optimizedGrid.GetOptimizedTileDataBase64();

        jData["ents"] = _entGrid.GetEntList();

        std::ofstream file(filePath);
        file << to_string(jData);

        if (file.fail()) return false;
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        return false;
    }

    return true;
}

bool MapMan::LoadTE3Map(fs::path filePath)
{
    _undoHistory.clear();
    _redoHistory.clear();
    
    using namespace nlohmann;

    std::ifstream file(filePath);
    json jData;
    file >> jData;

    try
    {
        auto tData = jData.at("tiles");
        
        //Replace our textures with the listed ones
        std::vector<std::string> texturePaths = tData.at("textures");
        _textureList.clear();
        _textureList.reserve(texturePaths.size());
        for (const auto& path : texturePaths)
        {
            _textureList.push_back(Assets::GetTexture(fs::path(path)));
        }

        //Same with models
        std::vector<std::string> shapePaths = tData.at("shapes");
        _modelList.clear();
        _modelList.reserve(shapePaths.size());
        for (const auto& path : shapePaths)
        {
            _modelList.push_back(Assets::GetModel(fs::path(path)));
        }

        _tileGrid = TileGrid(this, tData.at("width"), tData.at("height"), tData.at("length"), TILE_SPACING_DEFAULT, Tile());
        _tileGrid.SetTileDataBase64(tData.at("data"));
        
        _entGrid = EntGrid(_tileGrid.GetWidth(), _tileGrid.GetHeight(), _tileGrid.GetLength());
        for (const Ent& e : jData.at("ents").get<std::vector<Ent>>())
        {
            Vector3 gridPos = _entGrid.WorldToGridPos(e.position);
            _entGrid.AddEnt((int) gridPos.x, (int) gridPos.y, (int) gridPos.z, e);
        }
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        return false;
    }

    if (file.fail()) return false;

    return true;
}

static inline std::string GetDataURI(void *buffer, size_t byteCount)
{
    return std::string("data:application/octet-stream;base64,")
         + base64::encode(reinterpret_cast<uint8_t *>(buffer), byteCount);
} 

#define COMP_TYPE_FLOAT 5126
#define COMP_TYPE_UBYTE 5121

bool MapMan::ExportGLTFScene(fs::path filePath, bool separateGeometry)
{
    using namespace nlohmann;

    try
    {
        //The main JSON object that holds the entire document.
        json jData;
        jData["asset"] = {
            {"version", "2.0"},
            {"generator", "Total Editor 3"}
        };
        
        std::vector<json> scenes;
        std::vector<json> nodes;
        std::vector<json> meshes;
        std::vector<json> buffers;
        std::vector<json> bufferViews;
        std::vector<json> accessors;
        std::vector<json> materials;
        std::vector<json> textures;
        std::vector<json> images;

        //Automates the addition of buffers, bufferViews, and accessors for a given vertex attribute
        auto pushVertexAttrib = [&](int bufferIdx, std::string uri, size_t nBytes, size_t nElems, std::string elemType, int componentType) {
            buffers.push_back({
                {"byteLength", nBytes},
                {"uri", uri}
            });
            bufferViews.push_back({
                {"buffer", bufferIdx},
                {"byteLength", nBytes},
                {"byteOffset", 0},
                {"target", 34962} //ARRAY_BUFFER
            });
            accessors.push_back({
                {"bufferView", bufferIdx},
                {"byteOffset", 0},
                {"componentType", componentType},
                {"count", nElems},
                {"type", elemType}
            });
        };

        //Generate primitives and buffers for map geometry.
        const Model mapModel = _tileGrid.GetModel();
        std::vector<json> mapPrims;
        mapPrims.reserve(mapModel.meshCount);
        for (int i = 0; i < mapModel.meshCount; ++i)
        {
            //Each piece of vertex data gets its own buffer, bufferView, and accessor
            int posIdx = buffers.size();
            size_t posBytes = mapModel.meshes[i].vertexCount * sizeof(float) * 3;

            //Calculate max and min component values. Required only for position buffer.
            float minX, minY, minZ;
            minX = minY = minZ = std::numeric_limits<float>::max();
            float maxX, maxY, maxZ;
            maxX = maxY = maxZ = std::numeric_limits<float>::min();
            for (int j = 0; j < mapModel.meshes[i].vertexCount * 3; ++j)
            {
                const float C = mapModel.meshes[i].vertices[j];
                if (j % 3 == 0) //X
                {
                    if (C < minX) minX = C; if (C > maxX) maxX = C;
                }
                else if (j % 3 == 1) //Y
                {
                    if (C < minY) minY = C; if (C > maxY) maxY = C;
                }
                else if (j % 3 == 2) //Z
                {
                    if (C < minZ) minZ = C; if (C > maxZ) maxZ = C;
                }
            }

            pushVertexAttrib(
                posIdx, 
                GetDataURI((void *)mapModel.meshes[i].vertices, posBytes), 
                posBytes, 
                mapModel.meshes[i].vertexCount, 
                "VEC3", 
                COMP_TYPE_FLOAT
                );
            accessors[posIdx]["min"] = {minX, minY, minZ};
            accessors[posIdx]["max"] = {maxX, maxY, maxZ};

            //Tex coordinate buffer
            int texCoordIdx = buffers.size();
            size_t texCoordBytes = mapModel.meshes[i].vertexCount * sizeof(float) * 2;
            pushVertexAttrib(
                texCoordIdx, 
                GetDataURI((void *)mapModel.meshes[i].texcoords, texCoordBytes), 
                texCoordBytes, 
                mapModel.meshes[i].vertexCount, 
                "VEC2",
                COMP_TYPE_FLOAT
                );

            //Normal buffer
            int normalIdx = buffers.size();
            size_t normalBytes = mapModel.meshes[i].vertexCount * sizeof(float) * 3;
            pushVertexAttrib(
                normalIdx, 
                GetDataURI((void *)mapModel.meshes[i].normals, normalBytes), 
                normalBytes, 
                mapModel.meshes[i].vertexCount, 
                "VEC3",
                COMP_TYPE_FLOAT
                );

            // int colorIdx = buffers.size();
            // size_t colorBytes = mapModel.meshes[i].vertexCount * sizeof(unsigned char) * 4;
            // pushVertexAttrib(
            //     colorIdx, 
            //     GetDataURI((void *)mapModel.meshes[i].colors, colorBytes), 
            //     colorBytes, 
            //     mapModel.meshes[i].vertexCount, 
            //     "VEC4",
            //     COMP_TYPE_UBYTE
            //     );

            mapPrims.push_back({
                {"mode", 4}, //Triangles
                {"attributes", {
                    {"POSITION", posIdx},
                    {"TEXCOORD_0", texCoordIdx},
                    {"NORMAL", normalIdx}
                    // {"COLOR_0", colorIdx}
                }},
                {"material", i}
            });
        }

        //Encode materials and textures
        for (int m = 0; m < mapModel.materialCount; ++m)
        {
            materials.push_back({
                {"pbrMetallicRoughness", {
                    {"baseColorTexture", {
                        {"index", textures.size()},
                        {"texCoord", 0}
                    }}
                }}
            });

            textures.push_back({
                {"source", textures.size()}
            });

            fs::path imagePath = PathFromTexID(m);
            imagePath = fs::relative(
                fs::current_path() / imagePath, 
                fs::current_path() / filePath.parent_path()); //Image paths are relative to the file.

            images.push_back({
                {"uri", imagePath.c_str()}
            });
        }

        std::vector<int> rootNodes;

        //Assign map primitives to meshes, meshes to nodes.
        if (!separateGeometry)
        {
            json mapNode;
            json mapMesh;

            mapMesh["primitives"] = mapPrims;
            mapNode["mesh"] = meshes.size();
            mapNode["name"] = "map";
            meshes.push_back(mapMesh);
            rootNodes.push_back(nodes.size());
            nodes.push_back(mapNode);
        }
        else
        {
            //There will be a base node for all the map-related nodes.
            //Then, there will be children of that node for each material, which has all the map geometry with that material.

            json mapNode;
            std::vector<int> mapNodeChildren;

            for (int m = 0; m < mapModel.materialCount; ++m)
            {
                //Find texture name based off material
                Material mat = mapModel.materials[m];
                fs::path path = PathFromTexID(m);
                
                json materialNode;
                materialNode["name"] = path.generic_string();

                //Add children for each mesh with that material
                for (int mm = 0; mm < mapModel.meshCount; ++mm)
                {
                    if (mat.maps == mapModel.materials[mapModel.meshMaterial[mm]].maps)
                    {
                        json mesh = {
                            {"primitives", {mapPrims[mm]}}
                        };

                        materialNode["mesh"] = meshes.size();
                        meshes.push_back(mesh);
                        break;
                    }
                }
                if (materialNode.find("mesh") == materialNode.end())
                {
                    //Unknown materials get skipped
                    continue;
                }

                mapNodeChildren.push_back(nodes.size());
                nodes.push_back(materialNode);
            }

            mapNode["name"] = "map";
            mapNode["children"] = mapNodeChildren;
            rootNodes.push_back(nodes.size());
            nodes.push_back(mapNode);
        }

        //Add entities as nodes
        std::vector<Ent> ents = _entGrid.GetEntList();
        for (const Ent &ent : ents)
        {
            json entNode;
            if (ent.properties.find("name") != ent.properties.end())
            {
                entNode["name"] = ent.properties.at("name");
            }
            entNode["translation"] = { ent.position.x, ent.position.y, ent.position.z };
            entNode["scale"] = { 1.0f, 1.0f, 1.0f };
            Quaternion rot = QuaternionFromEuler(ToRadians((float)ent.pitch), ToRadians((float)ent.yaw), 0.0f);
            entNode["rotation"] = { rot.x, rot.y, rot.z, rot.w };
            entNode["extras"] = ent.properties;
            rootNodes.push_back(nodes.size());
            nodes.push_back(entNode);
        }

        //Set up scene. There is only one scene with all the nodes.
        json scene;
        scene["nodes"] = rootNodes;
        scenes.push_back(scene);
        jData["scene"] = 0;

        //Marshall all of the data into the main JSON object.
        jData["nodes"] = nodes;
        jData["meshes"] = meshes;
        jData["buffers"] = buffers;
        jData["bufferViews"] = bufferViews;
        jData["accessors"] = accessors;
        jData["scenes"] = scenes;
        jData["materials"] = materials;
        jData["textures"] = textures;
        jData["images"] = images;

        //Write JSON to file
        std::ofstream file(filePath);
        file << to_string(jData);

        if (file.fail()) return false;
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
    catch (...)
    {
        return false;
    }

    return true;
}

const std::vector<fs::path> MapMan::GetModelPathList() const 
{
    std::vector<fs::path> paths;
    for (const auto& handle : _modelList)
    {
        paths.push_back(handle->GetPath());
    }
    return paths;
}

const std::vector<fs::path> MapMan::GetTexturePathList() const 
{
    std::vector<fs::path> paths;
    for (const auto& handle : _textureList)
    {
        paths.push_back(handle->GetPath());
    }
    return paths;
}