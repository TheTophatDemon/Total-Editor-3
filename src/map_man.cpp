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

        // Save the modified tile data
        jData["tiles"]["data"] = optimizedGrid.GetOptimizedTileDataBase64();

        jData["ents"] = _entGrid.GetEntList();

        // Save camera orientation
        jData["editorCamera"] = json::object();
        jData["editorCamera"]["position"] = json::array({_defaultCameraPosition.x, _defaultCameraPosition.y, _defaultCameraPosition.z});
        jData["editorCamera"]["eulerAngles"] = json::array({_defaultCameraAngles.x * RAD2DEG, _defaultCameraAngles.y * RAD2DEG, _defaultCameraAngles.z * RAD2DEG});

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

void MapMan::DrawMap(Camera &camera, int fromY, int toY) 
{
    _tileGrid.Draw(Vector3Zero(), fromY, toY);
    _entGrid.Draw(camera, fromY, toY);
}