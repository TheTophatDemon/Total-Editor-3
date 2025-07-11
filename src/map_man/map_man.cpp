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

#include "../app.hpp"
#include "../assets.hpp"
#include "../text_util.hpp"

MapMan::MapMan()
    : _tileGrid(*this, 0, 0, 0)
{
    _numberOfChanges = 0;
}

void MapMan::NewMap(int width, int height, int length) 
{
    _tileGrid = TileGrid(*this, width, height, length);
    _entGrid = EntGrid(width, height, length);
    _undoHistory.clear();
    _redoHistory.clear();
}

void MapMan::DrawMap(Camera &camera, int fromY, int toY) 
{
    _tileGrid.Draw(Vector3Zero(), fromY, toY);
    _entGrid.Draw(camera, fromY, toY);
}

void MapMan::Draw2DElements(Camera &camera, int fromY, int toY)
{
    _entGrid.DrawLabels(camera, fromY, toY);
}

//Regenerates the map, extending one of the grid's dimensions on the given axis. Returns false if the change would result in an invalid map size.
void MapMan::ExpandMap(Direction axis, int amount)
{
    int newWidth  = _tileGrid.GetWidth();
    int newHeight = _tileGrid.GetHeight();
    int newLength = _tileGrid.GetLength();
    int ofsx, ofsy, ofsz;
    ofsx = ofsy = ofsz = 0;

    switch (axis)
    {
    case Direction::Z_NEG: newLength += amount; ofsz += amount; break;
    case Direction::Z_POS: newLength += amount; break;
    case Direction::X_NEG: newWidth += amount; ofsx += amount; break;
    case Direction::X_POS: newWidth += amount; break;
    case Direction::Y_NEG: newHeight += amount; ofsy += amount; break;
    case Direction::Y_POS: newHeight += amount; break;
    }

    _undoHistory.clear();
    _redoHistory.clear();
    TileGrid oldTiles = _tileGrid;
    EntGrid oldEnts = _entGrid;
    _tileGrid = TileGrid(*this, newWidth, newHeight, newLength);
    _entGrid = EntGrid(newWidth, newHeight, newLength);       
    _tileGrid.CopyTiles(ofsx, ofsy, ofsz, oldTiles, false);
    _entGrid.CopyEnts(ofsx, ofsy, ofsz, oldEnts);
}

//Reduces the size of the grid until it fits perfectly around all the non-empty cels in the map.
void MapMan::ShrinkMap()
{
    size_t minX, minY, minZ;
    size_t maxX, maxY, maxZ;
    minX = minY = minZ = std::numeric_limits<size_t>::max();
    maxX = maxY = maxZ = 0;
    for (size_t x = 0; x < _tileGrid.GetWidth(); ++x)
    {
        for (size_t y = 0; y < _tileGrid.GetHeight(); ++y)
        {
            for (size_t z = 0; z < _tileGrid.GetLength(); ++z)
            {
                if (_tileGrid.GetTile(x, y, z) || _entGrid.HasEnt(x, y, z))
                {
                    if (x < minX) minX = x;
                    if (y < minY) minY = y;
                    if (z < minZ) minZ = z;
                    if (x > maxX) maxX = x;
                    if (y > maxY) maxY = y;
                    if (z > maxZ) maxZ = z;
                }
            }
        }
    }
    if (minX > maxX || minY > maxY || minZ > maxZ)
    {
        //If there aren't any tiles, just make it 1x1x1.
        _tileGrid = TileGrid(*this, 1, 1, 1);
        _entGrid = EntGrid(1, 1, 1);
    }
    else
    {
        _tileGrid = _tileGrid.Subsection(minX, minY, minZ, maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
        _entGrid = _entGrid.Subsection(minX, minY, minZ, maxX - minX + 1, maxY - minY + 1, maxZ - minZ + 1);
    }
}

bool MapMan::SaveTE3Map(fs::path filePath)
{
    _undoHistory.clear();
    _redoHistory.clear();
    _willConvert = false;
    _numberOfChanges = 0;
    
    using namespace nlohmann;

    try
    {
        json jData;

        // Version information
        jData["meta"] = {{"editor", "Total Editor"}, {"version", "3.2"}};

        jData["tiles"] = json::object();
        jData["tiles"]["width"] = _tileGrid.GetWidth();
        jData["tiles"]["height"] = _tileGrid.GetHeight();
        jData["tiles"]["length"] = _tileGrid.GetLength();

        // Make new texture & model lists containing only used assets
        // This prevents extraneous assets from accumulating in the file every time it's saved
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

        // Make a copy of the map that reassigns all IDs to match the new lists
        const int tileArea = _tileGrid.GetWidth() * _tileGrid.GetHeight() * _tileGrid.GetLength();
        TileGrid optimizedGrid = _tileGrid.Subsection(0, 0, 0, _tileGrid.GetWidth(), _tileGrid.GetHeight(), _tileGrid.GetLength());
        for (int gridIdx = 0; gridIdx < tileArea; ++gridIdx)
        {
            Tile tile = optimizedGrid.GetTile(gridIdx);
            for (size_t t = 0; t < usedTexIDs.size(); ++t)
            {
                for (int i = 0; i < TEXTURES_PER_TILE; ++i)
                {
                    if (usedTexIDs[t] == tile.textures[i]) 
                    {
                        tile.textures[i] = (TexID)t;
                    }
                }
            }
            for (size_t m = 0; m < usedModelIDs.size(); ++m)
            {
                if (usedModelIDs[m] == tile.shape)
                {
                    tile.shape = m;
                    break;
                }
            }
            optimizedGrid.SetTile(gridIdx, tile);
        }

        // Save the modified tile data
        jData["tiles"]["data"] = optimizedGrid.GetTileDataBase64();

        jData["ents"] = _entGrid.GetEntList();

        // Save camera orientation
        jData["editorCamera"] = {
            {"position", {_defaultCameraPosition.x, _defaultCameraPosition.y, _defaultCameraPosition.z}},
            {"eulerAngles", {_defaultCameraAngles.x * RAD2DEG, _defaultCameraAngles.y * RAD2DEG, _defaultCameraAngles.z * RAD2DEG}},
        };

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
    _numberOfChanges = 0;

    using namespace nlohmann;

    std::ifstream file(filePath);
    json jData;
    file >> jData;

    try
    {
        int versionMajor = 3;
        int versionMinor = 1;
        if (jData.contains("meta") && jData["meta"].contains("version"))
        {
            std::string versionStr = jData["meta"]["version"];
            size_t periodLoc = versionStr.find('.');
            if (periodLoc == std::string::npos)
            {
                throw std::runtime_error("TE3 file version is invalid");
            }
            versionMajor = std::stoi(versionStr.substr(0, periodLoc));
            versionMinor = std::stoi(versionStr.substr(periodLoc + 1));
            _willConvert = false;
        }
        else
        {
            _willConvert = true;
        }

        json tiles = jData["tiles"];
        
        //Replace our textures with the listed ones
        std::vector<std::string> texturePaths = tiles["textures"];
        _textureList.clear();
        _textureList.reserve(texturePaths.size());
        for (const std::string& path : texturePaths)
        {
            _textureList.push_back(Assets::GetTexture(fs::path(path)));
        }

        //Same with models
        std::vector<std::string> shapePaths = tiles["shapes"];
        _modelList.clear();
        _modelList.reserve(shapePaths.size());
        for (const std::string& path : shapePaths)
        {
            _modelList.push_back(Assets::GetModel(fs::path(path)));
        }

        _tileGrid = TileGrid(
            *this, 
            (size_t) tiles["width"], 
            (size_t) tiles["height"], 
            (size_t) tiles["length"], 
            TILE_SPACING_DEFAULT, 
            Tile()
        );

        json tileData = tiles["data"];
        if (versionMajor >= 3 && versionMinor >= 2) 
        {
            _tileGrid.SetTileDataBase64(tileData);
        }
        else
        {
            _tileGrid.SetTileDataBase64OldFormat(tileData);
        }
        
        _entGrid = EntGrid(_tileGrid.GetWidth(), _tileGrid.GetHeight(), _tileGrid.GetLength());
        for (const Ent& e : jData["ents"].get<std::vector<Ent>>())
        {
            Vector3 gridPos = _entGrid.WorldToGridPos(e.lastRenderedPosition);
            _entGrid.AddEnt((int) gridPos.x, (int) gridPos.y, (int) gridPos.z, e);
        }

        if (jData.contains("editorCamera"))
        {
            json::array_t posArr = jData["editorCamera"]["position"];
            _defaultCameraPosition = Vector3 {
                (float)posArr[0], (float)posArr[1], (float)posArr[2]
            };
            json::array_t rotArr = jData["editorCamera"]["eulerAngles"];
            _defaultCameraAngles = Vector3 {
                (float)rotArr[0] * DEG2RAD, (float)rotArr[1] * DEG2RAD, (float)rotArr[2] * DEG2RAD
            };
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

TexID MapMan::GetOrAddTexID(const fs::path &texturePath) 
{
    //Look for existing ID
    for (size_t i = 0; i < _textureList.size(); ++i)
    {
        if (_textureList[i]->GetPath() == texturePath)
        {
            return (TexID)(i);
        }
    }
    //Create new ID and append texture to list
    TexID newID = _textureList.size();
    _textureList.push_back(Assets::GetTexture(texturePath));
    return newID;
}

ModelID MapMan::GetOrAddModelID(const fs::path &modelPath)
{
    //Look for existing ID
    for (size_t i = 0; i < _modelList.size(); ++i)
    {
        if (_modelList[i]->GetPath() == modelPath)
        {
            return (ModelID)(i);
        }
    }
    //Create new ID and append texture to list
    ModelID newID = _modelList.size();
    _modelList.push_back(Assets::GetModel(modelPath));
    return newID;
}

fs::path MapMan::PathFromTexID(const TexID id) const
{
    if (id == NO_TEX || (size_t)id >= _textureList.size()) return fs::path();
    return _textureList[id]->GetPath();
}

fs::path MapMan::PathFromModelID(const ModelID id) const
{
    if (id == NO_MODEL || (size_t)id >= _modelList.size()) return fs::path();
    return _modelList[id]->GetPath();
}

Model MapMan::ModelFromID(const ModelID id) const
{
    if (id == NO_MODEL || (size_t)id >= _modelList.size()) return Model{};
    return _modelList[id]->GetModel();
}

Texture MapMan::TexFromID(const TexID id) const
{
    if (id == NO_TEX || (size_t)id >= _textureList.size()) return Texture{};
    return _textureList[id]->GetTexture();
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

void MapMan::Undo()
{
    if (!_undoHistory.empty())
    {
        _undoHistory.back()->Undo(*this);
        _redoHistory.push_back(_undoHistory.back());
        _undoHistory.pop_back();
        --_numberOfChanges;
    }
}

void MapMan::Redo()
{
    if (!_redoHistory.empty())
    {
        _redoHistory.back()->Do(*this);
        _undoHistory.push_back(_redoHistory.back());
        _redoHistory.pop_back();
        ++_numberOfChanges;
    }
}

void MapMan::_Execute(std::shared_ptr<Action> action)
{
    _undoHistory.push_back(action);
    while (_undoHistory.size() > App::Get()->GetUndoMax()) 
    {
        _undoHistory.pop_front();
    }
    _redoHistory.clear();
    _undoHistory.back()->Do(*this);
    ++_numberOfChanges;
}