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

#include <fstream>
#include <iostream>
#include <limits>

#include "../app.hpp"
#include "../assets.hpp"
#include "../text_util.hpp"

#define TE2_FORMAT_ERR "ERROR: This is not a properly formatted .ti file."

bool MapMan::LoadTE2Map(fs::path filePath)
{
    _willConvert = true;
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
        
        // TILES (walls)
        std::getline(file, line);
        if (std::string_view(line).substr(0, 5).compare("TILES") != 0)
        {
            std::cerr << TE2_FORMAT_ERR << std::endl;
            return false;
        }
        std::getline(file, line);
        int tileCount = std::stoi(line);
        tilesToAdd.reserve(tileCount);
        for (int t = 0; t < tileCount; ++t)
        {
            std::getline(file, line);
            std::vector<std::string> tokens = SplitString(line, ",");
            
            // Original grid position (needs to be offset by minX/minZ)
            int i = std::stoi(tokens[0]) / 16;
            minX = Min(minX, i);
            maxX = Max(maxX, i);
            int k = std::stoi(tokens[1]) / 16;
            minZ = Min(minZ, k);
            maxZ = Max(maxZ, k);

            Tile tile;

            std::string textureName = tokens[3] + ".png";
            fs::path texturePath = fs::path(App::Get()->GetTexturesDir()) / textureName;
            
            int flag = std::stoi(tokens[4]);
            int link = std::stoi(tokens[5]);

            // Set angles for doors & panels
            if (flag == 7) 
            {
                tile.yaw = (link == 0) ? 0 : 90;
            }
            else if (flag == 2 || flag == 9) 
            {
                tile.yaw = 90;
            }
            else if (flag == 5)
            {
                tile.yaw = (90 + (link * 90)) % 360;
            }
            
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
            {
                tile.shape = barsID;
            }

            // Create entities for dynamic tiles
            if (flag > 0 && flag != 6 && flag != 7)
            {
                Ent ent = Ent(1.0f);
                ent.model = _modelList[tile.shape];
                ent.texture = Assets::GetTexture(texturePath);
                ent.yaw = tile.yaw;
                ent.pitch = tile.pitch;
                ent.display = Ent::DisplayMode::MODEL;
                switch (flag)
                {
                case 1: case 2: case 5: case 8: case 9: case 10: // Moving door like objects
                    ent.properties["type"] = "door";

                    if (link > 0) ent.properties["link"] = std::to_string(link);

                    switch (flag)
                    {
                    case 1: case 8: case 2: case 9: // Doors
                        ent.properties["direction"] = "right";
                        break;
                    case 5: // Push walls
                        ent.properties["direction"] = "backward"; break;
                        ent.properties["distance"] = "4.0";
                        ent.properties["wait"] = "inf";
                        ent.properties["activateSound"] = "secretwall.wav";
                        ent.properties.erase("link");
                        break;
                    case 10: // "Disappearing" walls
                        ent.properties["direction"] = "down";
                        ent.properties["distance"] = "4.0";
                        ent.properties["activateSound"] = "";
                        ent.properties["blockUse"] = "true";
                        ent.properties["wait"] = "inf";
                        break;
                    }

                    // Space doors move up instead
                    if (textureName.find("spacedoor") != std::string::npos) 
                    {
                        ent.properties["direction"] = "up";
                    }

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
                    
                    break;
                case 3:
                    ent.properties["type"] = "switch";
                    ent.properties["link"] = std::to_string(link);
                    break;
                case 4: case 11: case 12: case 13:
                    ent.properties["type"] = "trigger";
                    ent.properties["link"] = std::to_string(link);
                    switch (flag)
                    {
                    case 4: // Teleporter
                        ent.properties["action"] = "teleport";
                        break;
                    case 11: // Trigger
                        if (link == 255) 
                        {
                            ent.properties["action"] = "secret";
                            ent.properties.erase("link");
                        } 
                        else 
                        {
                            ent.properties["action"] = "activate";
                        }
                        break;
                    case 12: // Level exit
                        ent.properties["action"] = "end level";
                        ent.properties["level"] = "TODO!";
                        ent.properties.erase("link");
                        break;
                    case 13: // Conveyor belt
                        ent.properties["action"] = "push";
                        switch (link) 
                        {
                        case 0: ent.properties["direction"] = "forward"; break;
                        case 1: ent.properties["direction"] = "right"; break;
                        case 2: ent.properties["direction"] = "backward"; break;
                        case 3: ent.properties["direction"] = "left"; break;
                        }
                        ent.properties.erase("link");
                        break;
                    }
                    
                    break;
                }
                ent.properties["name"] = ent.properties["type"];
                tileEntities.push_back({i, 1, k, ent});
            }
            else
            {
                // Have to load the texture only when the tile isn't an entity, or the IDs will get messed up in the map model.
                TexID id = GetOrAddTexID(texturePath);
                for (int i = 0; i < TEXTURES_PER_TILE; ++i) tile.textures[i] = id;
                tilesToAdd.push_back({i, 1, k, tile});
            }
        }

        // SECTORS (floors and ceilings)
        std::getline(file, line);
        if (std::string_view(line).substr(0, 7).compare("SECTORS") != 0)
        {
            std::cerr << TE2_FORMAT_ERR << std::endl;
            return false;
        }
        std::getline(file, line);
        int floorCount = std::stoi(line);
        tilesToAdd.reserve(tileCount + floorCount);
        for (int t = 0; t < floorCount; ++t)
        {
            std::getline(file, line);
            std::vector<std::string> tokens = SplitString(line, ",");
            
            // Original grid position (needs to be offset by minX/minZ)
            int i = std::stoi(tokens[0]) / 16;
            minX = Min(minX, i);
            maxX = Max(maxX, i);
            int k = std::stoi(tokens[1]) / 16;
            minZ = Min(minZ, k);
            maxZ = Max(maxZ, k);

            Tile tile(cubeID, NO_TEX, NO_TEX, 0, 0);

            std::string textureName = tokens[5];
            textureName.append(".png");
            TexID texId = GetOrAddTexID(fs::path(App::Get()->GetTexturesDir()) / textureName);
            for (int i = 0; i < TEXTURES_PER_TILE; ++i) tile.textures[i] = texId;
            
            bool isCeiling = (bool)std::stoi(tokens[4]);
            
            tilesToAdd.push_back({i, isCeiling ? 2 : 0, k, tile});
        }
        
        // Calculate grid bounds
        size_t width = (maxX - minX) + 1;
        size_t length = (maxZ - minZ) + 1;

        // Fill tile grid
        _tileGrid = TileGrid(*this, width, 3, length, TILE_SPACING_DEFAULT, Tile());
        for (const auto& [i, j, k, tile] : tilesToAdd)
        {
            // Add the tiles to the grid, offset from the top left corner
            _tileGrid.SetTile(i - minX, j, k - minZ, tile);
        }
        
        // Get & convert entities
        _entGrid = EntGrid(_tileGrid.GetWidth(), _tileGrid.GetHeight(), _tileGrid.GetLength());
        std::getline(file, line);
        if (std::string_view(line).substr(0, 6).compare("THINGS") != 0)
        {
            std::cerr << TE2_FORMAT_ERR << std::endl;
            return false;
        }
        std::getline(file, line);

        int entCount = std::stoi(line);
        for (int e = 0; e < entCount; ++e)
        {
            std::getline(file, line);
            std::vector<std::string> tokens = SplitString(line, ",");

            size_t i = (std::stoul(tokens[0]) / 16) - minX;
            size_t k = (std::stoul(tokens[1]) / 16) - minZ;

            // Ignore out of bounds entities.
            if (i < 0 || k < 0 || i >= width || k >= length) 
            {
                TraceLog(LOG_WARNING, TextFormat("There is an entity out of bounds at position (%s, %s).", tokens[0].c_str(), tokens[1].c_str()));
                continue;
            }

            Ent ent = Ent(0.8f);
            ent.lastRenderedPosition = Vector3 { 
                (float)i * _tileGrid.GetSpacing(), 
                1.0f * _tileGrid.GetSpacing(), 
                (float)k * _tileGrid.GetSpacing() 
            };
            ent.yaw = 270 - (std::stoi(tokens[4]) * 45);
            
            int type = std::stoi(tokens[2]);
            switch(type)
            {
            case 0: // Player
                ent.color = BROWN;
                ent.properties["type"] = "player";
                ent.properties["name"] = "player";
                ent.display = Ent::DisplayMode::SPHERE;
                break;
            case 1: // Prop
                ent.properties["type"] = "prop";
                ent.properties["prop"] = tokens[3];
                ent.properties["name"] = tokens[3];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/" + tokens[3] + ".png");
                break;
            case 2: // Item
                ent.properties["type"] = "item";
                ent.properties["item"] = tokens[3];
                ent.properties["name"] = tokens[3];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/" + tokens[3] + ".png");
                break;
            case 3: // Weapon
                ent.properties["type"] = "weapon";
                ent.properties["weapon"] = tokens[3];
                ent.properties["name"] = tokens[3];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/" + tokens[3] + ".png");
                break;
            case 4: // Wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/wraith_icon.png");
                break;
            case 5: // Fire Wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "fire wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/firewraith_icon.png");
                break;
            case 6: // Dummkopf
            case 14: // Dummkopf (disguised)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "dummkopf";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                if (type == 14)
                {
                    ent.properties["disguised"] = "true";
                    ent.texture = Assets::GetTexture("assets/textures/icons/dummkopf_pawn_icon.png");
                }
                else
                {
                    ent.texture = Assets::GetTexture("assets/textures/icons/dummkopf_icon.png");
                }
                break;
            case 7: // Caco wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "mother wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/cacowraith_icon.png");
                break;
            case 8: // Prisrak
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "prisrak";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/prisrak_icon.png");
                break;
            case 9: // Providence (first boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "providence";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/providence_icon.png");
                break;
            case 10: // Fundie (secret level)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "fundie";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/fundie_icon.png");
                break;
            case 11: // Banshee
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "banshee";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/banshee_icon.png");
                break;
            case 12: // Mutant wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "mutant wraith";
                ent.properties["name"] = ent.properties["enemy"];
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/mutant_icon.png");
                break;
            case 13: // Mecha (second boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "mecha";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/mecha_icon.png");
                break;
            case 15: // Tophat demon (final boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "tophat demon";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/demon_icon.png");
                break;
            }

            if (ent.display == Ent::DisplayMode::SPRITE && ent.texture->GetTexture().id == Assets::GetMissingTexture().id)
            {
                // Default to sphere if the sprite didn't load
                ent.display = Ent::DisplayMode::SPHERE;
            }

            _entGrid.AddEnt(i, 1, k, ent);
        }

        // Insert tile entities
        for (const auto& [i, j, k, ent] : tileEntities)
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