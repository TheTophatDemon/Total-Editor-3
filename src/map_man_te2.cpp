#include "map_man.hpp"

#include <fstream>
#include <iostream>
#include <limits>

#include "app.hpp"
#include "assets.hpp"
#include "text_util.hpp"

#define TE2_FORMAT_ERR "ERROR: This is not a properly formatted .ti file."

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
            fs::path texturePath = fs::path(App::Get()->GetTexturesDir()) / textureName;
            
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
                Ent ent = Ent(1.0f);
                ent.model = _modelList[tile.shape];
                ent.texture = Assets::GetTexture(texturePath);
                ent.yaw = tile.angle;
                ent.pitch = tile.pitch;
                ent.display = Ent::DisplayMode::MODEL;
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
                        ent.properties["key"] = std::to_string(link);
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
                    
                    break;
                case 3:
                    ent.properties["type"] = "switch";
                    ent.properties["destination"] = std::to_string(link);
                    
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
                    
                    break;
                }
                ent.properties["name"] = ent.properties["type"];
                tileEntities.push_back({i, 1, k, ent});
            }
            else
            {
                // Have to load the texture only when the tile isn't an entity, or the IDs will get messed up in the map model.
                tile.texture = GetOrAddTexID(texturePath);
                tilesToAdd.push_back({i, 1, k, tile});
            }
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

            Ent ent = Ent(1.0f);
            ent.position = Vector3 { 
                (float)i * _tileGrid.GetSpacing(), 
                1.0f * _tileGrid.GetSpacing(), 
                (float)k * _tileGrid.GetSpacing() 
            };
            ent.yaw = atoi(tokens[4].c_str()) * 45 + 90;
            
            int type = atoi(tokens[2].c_str());
            switch(type)
            {
            case 0: // Player
                // ent.color = BROWN;
                ent.properties["type"] = "player";
                ent.properties["name"] = "player";
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/segan_icon.png");
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
                // ent.color = ORANGE;
                // ent.radius = 0.5f;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/" + tokens[3] + ".png");
                break;
            case 4: // Wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "wraith";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = BLUE;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/wraith_icon.png");
                break;
            case 5: // Fire Wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "fire wraith";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = RED;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/firewraith_icon.png");
                break;
            case 6: // Dummkopf
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "dummkopf";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = PURPLE;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/dummkopf_icon.png");
                break;
            case 7: // Caco wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "caco wraith";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = Color { 41, 120, 255, 255 }; // Teal-ish
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/cacowraith_icon.png");
                break;
            case 8: // Prisrak
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "prisrak";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = ORANGE;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/prisrak_icon.png");
                break;
            case 9: // Providence (first boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "providence";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                // ent.color = YELLOW;
                // ent.radius = 2.0f;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/providence_icon.png");
                break;
            case 10: // Fundie (secret level)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "fundie";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = BROWN;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/fundie_icon.png");
                break;
            case 11: // Banshee
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "banshee";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = DARKGRAY;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/banshee_icon.png");
                break;
            case 12: // Mutant wraith
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "mutant wraith";
                ent.properties["name"] = ent.properties["enemy"];
                // ent.color = GRAY;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/mutant_icon.png");
                break;
            case 13: // Mecha (second boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "mecha";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                // ent.color = RED;
                // ent.radius = 2.0f;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/mecha_icon.png");
                break;
            case 14: // Dummkopf (disguised)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "dummkopf";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["disguised"] = "true";
                // ent.color = PURPLE;
                ent.display = Ent::DisplayMode::SPRITE;
                ent.texture = Assets::GetTexture("assets/textures/icons/dummkopf_pawn_icon.png");
                break;
            case 15: // Tophat demon (final boss)
                ent.properties["type"] = "enemy";
                ent.properties["enemy"] = "tophat demon";
                ent.properties["name"] = ent.properties["enemy"];
                ent.properties["boss"] = "true";
                // ent.color = DARKPURPLE;
                // ent.radius = 4.0f;
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