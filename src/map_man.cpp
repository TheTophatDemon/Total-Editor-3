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
#include "text_util.hpp"

#define TE2_FORMAT_ERR "ERROR: This is not a properly formatted .ti file."

// ======================================================================
// TILE ACTION
// ======================================================================

MapMan::TileAction::TileAction(size_t i, size_t j, size_t k, TileGrid prevState, TileGrid newState)
    : _i(i), _j(j), _k(k), 
    _prevState(prevState), 
    _newState(newState)
{}

void MapMan::TileAction::Do(MapMan& map) const
{
    map._tileGrid.CopyTiles(_i, _j, _k, _newState);
}

void MapMan::TileAction::Undo(MapMan& map) const
{
    map._tileGrid.CopyTiles(_i, _j, _k, _prevState);
}

// ======================================================================
// ENT ACTION
// ======================================================================

MapMan::EntAction::EntAction(size_t i, size_t j, size_t k, bool overwrite, bool removed, Ent oldEnt, Ent newEnt)
    : _i(i), _j(j), _k(k), 
    _overwrite(overwrite), 
    _removed(removed), 
    _oldEnt(oldEnt), 
    _newEnt(newEnt)
{}

void MapMan::EntAction::Do(MapMan& map) const
{
    if (_removed)
    {
        map._entGrid.RemoveEnt(_i, _j, _k);
    }
    else
    {
        map._entGrid.AddEnt(_i, _j, _k, _newEnt);
    }
}

void MapMan::EntAction::Undo(MapMan& map) const
{
    if (_overwrite || _removed)
    {
        map._entGrid.AddEnt(_i, _j, _k, _oldEnt);
    }
    else
    {
        map._entGrid.RemoveEnt(_i, _j, _k);
    }
}

// ======================================================================
// MAP MAN
// ======================================================================

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
    Ent prevEnt = _entGrid.HasEnt(i, j, k) ? _entGrid.GetEnt(i, j, k) : Ent();
    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<EntAction>(i, j, k, _entGrid.HasEnt(i, j, k), false, prevEnt, newEnt)
    ));
}

void MapMan::ExecuteEntRemoval(int i, int j, int k)
{
    _Execute(std::static_pointer_cast<Action>(
        std::make_shared<EntAction>(i, j, k, true, true, _entGrid.GetEnt(i, j, k), Ent())
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

bool MapMan::LoadTE2Map(fs::path filePath)
{
    _undoHistory.clear();
    _redoHistory.clear();

    std::ifstream file(filePath);

    try
    {
        _textureList.clear();

        _modelList.clear();
        ModelID cubeID = GetOrAddModelID(fs::path(App::Get()->GetShapesDir()) / "cube.obj");
        ModelID panelID = GetOrAddModelID(fs::path(App::Get()->GetShapesDir()) / "panel.obj");
        ModelID barsID = GetOrAddModelID(fs::path(App::Get()->GetShapesDir()) / "bars.obj");

        // Since the .ti format has no specific grid size (or origin), we must keep track of the map's extents manually.
        int minX, minZ, maxX, maxZ;
        minX = minZ = std::numeric_limits<int>::max();
        maxX = maxZ = std::numeric_limits<int>::min();

        // Stores tiles to add along with their grid positions (i, j, k) after the extents are calculated.
        std::vector<std::tuple<int, int, int, Tile>> tilesToAdd;
        // Stores entities that are placed inside of "dynamic" tiles to give them behavior.
        std::vector<std::tuple<int, int, int, Ent>> tileEntities;

        // Parse the text file line by line
        std::string line;
        
        //TILES (walls)
        std::getline(file, line);
        if (line.compare("TILES") != 0)
        {
            std::cerr << TE2_FORMAT_ERR << std::endl;
            return false;
        }
        std::getline(file, line);
        int tileCount = atoi(line.c_str());
        tilesToAdd.reserve(tileCount);
        for (int t = 0; t < tileCount; ++t)
        {
            std::getline(file, line);
            std::vector<std::string> tokens = SplitString(line, ",");
            
            // Original grid position (needs to be offset by minX/minZ)
            int i = atoi(tokens[0].c_str()) / 16;
            minX = Min(minX, i);
            maxX = Max(maxX, i);
            int k = atoi(tokens[1].c_str()) / 16;
            minZ = Min(minZ, k);
            maxZ = Max(maxZ, k);

            Tile tile(NO_MODEL, 0, NO_TEX, 0);

            std::string textureName = tokens[3] + ".png";
            tile.texture = GetOrAddTexID(fs::path(App::Get()->GetTexturesDir()) / textureName);
            
            int flag = atoi(tokens[4].c_str());
            int link = atoi(tokens[5].c_str());

            // Set angles for doors & panels
            if (flag == 7)
                tile.angle = (link == 0) ? 0 : 90;
            if (flag == 2 || flag == 9)
                tile.angle = 90;
            
            // Set shape depending on tile type
            switch (flag)
            {
            case 1: case 2: case 7: case 8: case 9:
                tile.shape = panelID;
                break;
            default:
                tile.shape = cubeID;
                break;
            }
            if (textureName.find("bars") != std::string::npos)
                tile.shape = barsID;

            // Create entities for dynamic tiles
            if (flag > 0 && flag != 6 && flag != 7)
            {
                Ent ent = Ent(0.5f);
                switch (flag)
                {
                case 1: case 2: case 5: case 8: case 9: case 10: // Moving door like objects
                    ent.properties["type"] = "door";

                    switch (flag)
                    {
                    case 1: case 8: // Horizontal doors
                        ent.properties["direction"] = std::to_string(0);
                        break;
                    case 2: case 9: // Vertical doors
                        ent.properties["direction"] = std::to_string(90);
                        break;
                    case 5: // Push walls
                        ent.properties["direction"] = std::to_string(link * 90);
                        break;
                    case 10: // "Disappearing" walls
                        ent.properties["direction"] = "down";
                        break;
                    }

                    // Space doors move up instead
                    if (textureName.find("spacedoor") != std::string::npos)
                        ent.properties["direction"] = "up";

                    ent.properties["distance"] = std::to_string((flag == 5 || flag == 10) ? 4.0f : 1.8f);

                    if (flag == 8 || flag == 9) // Locked doors
                    {
                        switch(link)
                        {
                        case 0: ent.properties["key"] = "blue"; break;
                        case 1: ent.properties["key"] = "brown"; break;
                        case 2: ent.properties["key"] = "yellow"; break;
                        case 3: ent.properties["key"] = "gray"; break;
                        }
                    }
                    ent.color = BROWN;

                    break;
                case 3:
                    ent.properties["type"] = "switch";
                    ent.properties["destination"] = std::to_string(link);
                    ent.color = SKYBLUE;
                    break;
                case 4: case 11: case 12: case 13:
                    ent.properties["type"] = "trigger";

                    switch (flag)
                    {
                    case 4:
                        ent.properties["action"] = "teleport";
                        ent.properties["destination"] = std::to_string(link);
                        break;
                    case 11:
                        ent.properties["action"] = "activate";
                        ent.properties["destination"] = (link == 255) ? "secret" : std::to_string(link);
                        break;
                    case 12:
                        ent.properties["action"] = "end level";
                        ent.properties["destination"] = (link == 255) ? "secret" : "next";
                        break;
                    case 13:
                        ent.properties["action"] = "push";
                        ent.properties["direction"] = std::to_string(link * 90);
                        break;
                    }
                    ent.color = MAGENTA;
                    break;
                }
                tileEntities.push_back({i, 1, k, ent});
            }
            
            tilesToAdd.push_back({i, 1, k, tile});
        }

        //SECTORS (floors and ceilings)
        std::getline(file, line);
        if (line.compare("SECTORS") != 0)
        {
            std::cerr << TE2_FORMAT_ERR << std::endl;
            return false;
        }
        std::getline(file, line);
        int floorCount = atoi(line.c_str());
        tilesToAdd.reserve(tileCount + floorCount);
        for (int t = 0; t < floorCount; ++t)
        {
            std::getline(file, line);
            std::vector<std::string> tokens = SplitString(line, ",");
            
            // Original grid position (needs to be offset by minX/minZ)
            int i = atoi(tokens[0].c_str()) / 16;
            minX = Min(minX, i);
            maxX = Max(maxX, i);
            int k = atoi(tokens[1].c_str()) / 16;
            minZ = Min(minZ, k);
            maxZ = Max(maxZ, k);

            Tile tile(cubeID, 0, NO_TEX, 0);

            std::string textureName = tokens[5];
            textureName.append(".png");
            tile.texture = GetOrAddTexID(fs::path(App::Get()->GetTexturesDir()) / textureName);
            
            bool isCeiling = (bool)atoi(tokens[4].c_str());
            
            tilesToAdd.push_back({i, isCeiling ? 2 : 0, k, tile});
        }
        
        // Calculate grid bounds
        size_t width = (maxX - minX) + 1;
        size_t length = (maxZ - minZ) + 1;

        // Fill tile grid
        _tileGrid = TileGrid(this, width, 3, length, TILE_SPACING_DEFAULT, Tile());
        for (const auto[i, j, k, tile] : tilesToAdd)
        {
            // Add the tiles to the grid, offset from the top left corner
            _tileGrid.SetTile(i - minX, j, k - minZ, tile);

        }
        
        // Get & convert entities
        _entGrid = EntGrid(_tileGrid.GetWidth(), _tileGrid.GetHeight(), _tileGrid.GetLength());
        std::getline(file, line);
        if (line.compare("THINGS") != 0)
        {
            std::cerr << TE2_FORMAT_ERR << std::endl;
            return false;
        }
        std::getline(file, line);
        int entCount = atoi(line.c_str());
        for (int e = 0; e < entCount; ++e)
        {
            std::getline(file, line);
            std::vector<std::string> tokens = SplitString(line, ",");

            int i = (atoi(tokens[0].c_str()) / 16) - minX;
            int k = (atoi(tokens[1].c_str()) / 16) - minZ;
            // Ignore out of bounds entities. I don't think the original game even has any...?
            if (i < 0 || k < 0 || i >= width || k >= length)
                continue;

            Ent ent = Ent();
            ent.position = Vector3 { 
                (float)i * _tileGrid.GetSpacing(), 
                1.0f * _tileGrid.GetSpacing(), 
                (float)k * _tileGrid.GetSpacing() 
            };
            ent.yaw = atoi(tokens[4].c_str()) * 45 + 90;
            ent.pitch = 0;
            ent.radius = 1.0f;
            ent.color = WHITE;
            ent.properties = std::map<std::string, std::string>();
            
            int type = atoi(tokens[2].c_str());
            switch(type)
            {
            case 0: // Player
                ent.color = BROWN;
                ent.properties["type"] = "player";
                ent.properties["name"] = "player";
                break;
            case 1: // Prop
                ent.properties["type"] = "prop";
                ent.properties["prop"] = tokens[3];
                ent.properties["name"] = tokens[3];
                break;
            case 2: // Item
                ent.properties["type"] = "item";
                ent.properties["item"] = tokens[3];
                ent.properties["name"] = tokens[3];
                ent.color = YELLOW;
                ent.radius = 0.5f;
                break;
            case 3: // Weapon
                ent.properties["type"] = "weapon";
                ent.properties["weapon"] = tokens[3];
                ent.properties["name"] = tokens[3];
                ent.color = ORANGE;
                ent.radius = 0.5f;
                break;
            case 4: // Wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = BLUE;
                break;
            case 5: // Fire Wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "fire wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = RED;
                break;
            case 6: // Dummkopf
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "dummkopf";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = PURPLE;
                break;
            case 7: // Caco wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "caco wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = Color { 41, 120, 255, 255 }; // Teal-ish
                break;
            case 8: // Prisrak
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "prisrak";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = ORANGE;
                break;
            case 9: // Providence (first boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "providence";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                ent.color = YELLOW;
                ent.radius = 2.0f;
                break;
            case 10: // Fundie (secret level)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "fundie";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = BROWN;
                break;
            case 11: // Banshee
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "banshee";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = DARKGRAY;
                break;
            case 12: // Mutant wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "mutant wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.color = GRAY;
                break;
            case 13: // Mecha (second boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "mecha";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                ent.color = RED;
                ent.radius = 2.0f;
                break;
            case 14: // Dummkopf (disguised)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "dummkopf";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["disguised"] = "true";
                ent.color = PURPLE;
                break;
            case 15: // Tophat demon (final boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "tophat demon";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                ent.color = DARKPURPLE;
                ent.radius = 4.0f;
                break;
            }

            _entGrid.AddEnt(i, 1, k, ent);
        }

        // Insert tile entities
        for (const auto[i, j, k, ent] : tileEntities)
        {
            _entGrid.AddEnt(i - minX, j, k - minZ, ent);
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