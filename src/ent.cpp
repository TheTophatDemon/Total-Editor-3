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

#define DISPLAY_NAME_THRESHOLD 0.9995f

// Default, inactive entity
Ent::Ent()
{
    active = false;
    display = DisplayMode::SPHERE;
    color = WHITE;
    radius = 0.0f;
    position = Vector3Zero();
    yaw = pitch = 0;
    properties = std::map<std::string, std::string>();
    model = nullptr;
    texture = nullptr;
}

Ent::Ent(float radius)
{
    active = true;
    display = DisplayMode::SPHERE;
    color = WHITE;
    this->radius = radius;
    position = Vector3Zero();
    yaw = pitch = 0;
    properties = std::map<std::string, std::string>();
    model = nullptr;
    texture = nullptr;
}

inline Matrix Ent::GetMatrix() const
{
    return MatrixMultiply(
        MatrixMultiply(
            MatrixRotateX(ToRadians((float) pitch)), 
            MatrixRotateY(ToRadians((float) yaw))
        ),
        MatrixTranslate(position.x, position.y, position.z));
}

void Ent::Draw(const bool drawAxes) const
{
    // All sprites share the same material, and the texture is changed for each one
    static Material spriteMaterial = LoadMaterialDefault();

    Matrix matrix = GetMatrix();

    switch (display)
    {
    case DisplayMode::SPHERE:
        {
            DrawModel(Assets::GetEntSphere(), position, radius, color);
            break;
        }

    case DisplayMode::MODEL:
        {
            if (model == nullptr) break;
            
            //Set up material
            Material mat = LoadMaterialDefault();
            if (texture != nullptr) SetMaterialTexture(&mat, MATERIAL_MAP_ALBEDO, texture->GetTexture());
            mat.shader = Assets::GetMapShader(false);
            mat.maps[MATERIAL_MAP_ALBEDO].color = color;
            
            Model mod = model->GetModel();
            mod.transform = MatrixMultiply(mod.transform, 
                MatrixMultiply(MatrixScale(radius, radius, radius), matrix));
            for (int m = 0; m < mod.meshCount; ++m)
            {
                DrawMesh(mod.meshes[m], mat, mod.transform);
            }

            RL_FREE(mat.maps);
            break;
        }

    case DisplayMode::SPRITE:
        {
            if (texture == nullptr) break;
            
            rlDrawRenderBatchActive();
            spriteMaterial.shader = Assets::GetSpriteShader();
            SetMaterialTexture(&spriteMaterial, MATERIAL_MAP_ALBEDO, texture->GetTexture());
            spriteMaterial.maps[MATERIAL_MAP_ALBEDO].color = color;
            DrawMesh(Assets::GetSpriteQuad(), spriteMaterial, MatrixMultiply(MatrixScale(radius, radius, radius), matrix));
            break;
        }
    }
    
    if (drawAxes)
    {
        //Draw axes to show orientation
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(matrix));
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
    j["display"] = ent.display;
    if (ent.model != nullptr) j["model"] = ent.model->GetPath();
    if (ent.texture != nullptr) j["texture"] = ent.texture->GetPath();
    j["properties"] = ent.properties;
}

void from_json(const nlohmann::json& j, Ent &ent)
{
    ent.active = true;
    ent.radius = j.at("radius");
    ent.color = Color { j.at("color").at(0), j.at("color").at(1), j.at("color").at(2), 255 };
    ent.position = Vector3 { j.at("position").at(0), j.at("position").at(1), j.at("position").at(2) };
    ent.pitch = j.at("angles").at(0);
    ent.yaw = j.at("angles").at(1);
    ent.properties = j.at("properties");

    if (j.contains("display"))
        ent.display = j.at("display");
    else 
        ent.display = Ent::DisplayMode::SPHERE;
    
    switch (ent.display)
    {
    case Ent::DisplayMode::MODEL:
        if (j.contains("model")) ent.model = Assets::GetModel(j.at("model"));
        //fallthrough
    case Ent::DisplayMode::SPRITE:
        if (j.contains("texture")) ent.texture = Assets::GetTexture(j.at("texture"));
        break;
    }
}

EntGrid::EntGrid()
    : EntGrid(0, 0, 0)
{
}

//Creates ent grid of given dimensions, default spacing.
EntGrid::EntGrid(size_t width, size_t height, size_t length)
    : Grid<Ent>(width, height, length, ENT_SPACING_DEFAULT, Ent())
{
    _labelsToDraw.reserve(_grid.size());
}

void EntGrid::Draw(Camera &camera, int fromY, int toY)
{
    _labelsToDraw.clear();

    for (size_t i = fromY * _width * _length; i < (toY + 1) * _width * _length; ++i)
    {
        if (!_grid[i].active) continue;

        //Do frustrum culling check
        Vector3 ndc = GetWorldToNDC(_grid[i].position, camera);
        if (ndc.z < 1.0f && ndc.x > -1.0f && ndc.x < 1.0f && ndc.y > -1.0f && ndc.y < 1.0f)
        {
            bool drawExtras = (ndc.z < DISPLAY_NAME_THRESHOLD);

            if (drawExtras && _grid[i].properties.find("name") != _grid[i].properties.end())
                _labelsToDraw.push_back(std::make_pair(ndc, _grid[i].properties["name"]));
            
            _grid[i].Draw(drawExtras && !App::Get()->IsPreviewing());
        }
    }
}

void EntGrid::DrawLabels(Camera &camera, int fromY, int toY)
{
    if (App::Get()->IsPreviewing()) return;
    
    for (const auto& [ndc, name] : _labelsToDraw)
    {
        float fontSize = Assets::GetFont().baseSize;
        Vector2 projectPos = Vector2{ (float)GetScreenWidth() * (ndc.x + 1.0f) / 2.0f, (float)GetScreenHeight() * (ndc.y + 1.0f) / 2.0f };
        int stringWidth = GetStringWidth(Assets::GetFont(), fontSize, name);

        float labelX = projectPos.x - (float)stringWidth / 2.0f;
        float labelY = projectPos.y - fontSize / 2.0f;

        DrawRectangle((int)labelX, (int)labelY, (float)stringWidth, fontSize, BLACK);
        DrawTextEx(Assets::GetFont(), name.c_str(), Vector2 { labelX, labelY }, fontSize, 0.0f, WHITE);
    }
}
