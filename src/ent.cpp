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

#include "ent.hpp"

#include "raymath.h"
#include "rlgl.h"

#include <iostream>

#include "app.hpp"
#include "draw_extras.h"
#include "text_util.hpp"
#include "assets.hpp"
#include "map_man.hpp"

void EntGrid::Draw(Camera &camera, int fromY, int toY)
{
    for (size_t i = 0; i < _grid.size(); ++i)
    {
        if (_grid[i])
        {
            Vector3 gridCoords = UnflattenIndex(i);
            if (gridCoords.y >= (float)fromY - 0.1f && gridCoords.y <= toY + 0.1f) 
            {
                _grid[i].position = GridToWorldPos(gridCoords, true);
                //Do frustrum culling check
                Vector3 ndc = GetWorldToNDC(_grid[i].position, camera);
                if (ndc.z < 1.0f && ndc.x > -1.0f && ndc.x < 1.0f && ndc.y > -1.0f && ndc.y < 1.0f)
                {
                    _grid[i].Draw();
                }
            }
        }
    }
}

void EntGrid::DrawLabels(Camera &camera, int fromY, int toY)
{
    if (!App::Get()->IsPreviewing())
    {
        for (size_t i = 0; i < _grid.size(); ++i)
        {
            if (_grid[i])
            {
                Vector3 gridCoords = UnflattenIndex(i);
                if (gridCoords.y >= (float)fromY - 0.1f && gridCoords.y <= toY + 0.1f) 
                {
                    if (_grid[i].properties.find("name") != _grid[i].properties.end()) 
                    {
                        //Do frustrum culling check
                        Vector3 ndc = GetWorldToNDC(_grid[i].position, camera);
                        if (ndc.z < 0.9995f)
                        {
                            // std::cout << ndc.z << std::endl;
                            //Draw label
                            std::string name = _grid[i].properties.at("name");

                            float fontSize = Assets::GetFont().baseSize;
                            Vector2 projectPos = Vector2{ (float)GetScreenWidth() * (ndc.x + 1.0f) / 2.0f, (float)GetScreenHeight() * (ndc.y + 1.0f) / 2.0f };
                            int stringWidth = GetStringWidth(Assets::GetFont(), fontSize, name);

                            float labelX = projectPos.x - (float)stringWidth / 2.0f;
                            float labelY = projectPos.y - fontSize / 2.0f;

                            DrawRectangle((int)labelX, (int)labelY, (float)stringWidth, fontSize, BLACK);
                            DrawTextEx(Assets::GetFont(), name.c_str(), Vector2 { labelX, labelY }, fontSize, 0.0f, WHITE);
                        }
                    }
                }
            }
        }
    }
}

void Ent::Draw() const
{
    //Draw sphere
    // DrawSphere(position, radius, color);
    DrawModel(Assets::GetEntSphere(), position, radius, color);
    if (!App::Get()->IsPreviewing())
    {
        //Draw axes to show orientation
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(GetMatrix()));
        DrawAxes3D(Vector3Zero(), radius);
        rlPopMatrix();
    }
}

void to_json(nlohmann::json& j, const Ent &ent)
{
    j["radius"] = ent.radius;
    j["color"] = nlohmann::json::array({ent.color.r, ent.color.g, ent.color.b});
    j["position"] = nlohmann::json::array({ent.position.x, ent.position.y, ent.position.z});
    j["angles"] = nlohmann::json::array({ent.pitch, ent.yaw, 0.0f});
    j["properties"] = ent.properties;
}

void from_json(const nlohmann::json& j, Ent &ent)
{
    ent.radius = j.at("radius");
    ent.color = Color { j.at("color").at(0), j.at("color").at(1), j.at("color").at(2), 255 };
    ent.position = Vector3 { j.at("position").at(0), j.at("position").at(1), j.at("position").at(2) };
    ent.pitch = j.at("angles").at(0);
    ent.yaw = j.at("angles").at(1);
    ent.properties = j.at("properties");
}
