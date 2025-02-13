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

#include "place_mode.hpp"

#include "../assets.hpp"
#include "../math_stuff.hpp"

PlaceMode::TileCursor::TileCursor() 
{
    model = nullptr;
    for (auto& tex : textures) tex = nullptr;
    yaw = pitch = 0;
    for (auto& material : materials) 
    {
        material = LoadMaterialDefault();
        material.shader = Assets::GetMapShader(false);
    }
}

void PlaceMode::TileCursor::Update(MapMan& mapMan) 
{
    bool multiSelect = IsKeyDown(KEY_LEFT_SHIFT);
    if (!multiSelect)
    {
        _tileCursor.endPosition = _tileCursor.position;
    }

    // Perform Tile operations
    Vector3 cursorStartGridPos = mapMan.Tiles().WorldToGridPos(position);
    Vector3 cursorEndGridPos = mapMan.Tiles().WorldToGridPos(endPosition);
    Vector3 gridPosMin = Vector3Min(cursorStartGridPos, cursorEndGridPos);
    Vector3 gridPosMax = Vector3Max(cursorStartGridPos, cursorEndGridPos);
    size_t i = (size_t)gridPosMin.x; 
    size_t j = (size_t)gridPosMin.y;
    size_t k = (size_t)gridPosMin.z;
    size_t w = (size_t)gridPosMax.x - i + 1;
    size_t h = (size_t)gridPosMax.y - j + 1;
    size_t l = (size_t)gridPosMax.z - k + 1;
    Tile underTile = mapMan.Tiles().GetTile(i, j, k); // * hi. my name's sans undertile...you get it?

    // Rotate cursor
    if (IsKeyPressed(KEY_Q))
    {
        yaw = OffsetDegrees(yaw, -90);
    }
    else if (!IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
    {
        yaw = OffsetDegrees(yaw, 90);
    }
    if (IsKeyPressed(KEY_F))
    {
        pitch = OffsetDegrees(pitch, 90);
    }
    else if (IsKeyPressed(KEY_V))
    {
        pitch = OffsetDegrees(pitch, -90);
    }
    // Reset tile orientation
    if (IsKeyPressed(KEY_R))
    {
        yaw = pitch = 0;
    }

    Tile cursorTile = Tile(
        mapMan.GetOrAddModelID(model->GetPath()),
        mapMan.GetOrAddTexID(textures[0]->GetPath()),
        mapMan.GetOrAddTexID(textures[1]->GetPath()),
        yaw,
        pitch
    );

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_ALT) && !multiSelect) 
    {
        // Place tiles
        if (underTile != cursorTile)
        {
            mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, cursorTile);
        }
    }
    else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && multiSelect)
    {
        // Place tiles rectangle
        mapMan.ExecuteTileAction(i, j, k, w, h, l, cursorTile);
    }
    else if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !multiSelect && underTile) 
    {
        // Remove tiles
        mapMan.ExecuteTileAction(i, j, k, 1, 1, 1, Tile());
    } 
    else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT) && multiSelect)
    {
        // Remove tiles RECTANGLE
        mapMan.ExecuteTileAction(i, j, k, w, h, l, Tile());
    }
    else if (IsKeyPressed(KEY_G) && !multiSelect && underTile) 
    {
        // (G)rab the shape from the tile under the cursor
        fs::path path = mapMan.PathFromModelID(underTile.shape);
        model = Assets::GetModel(path);
        yaw = underTile.yaw;
        pitch = underTile.pitch;
    } 
    else if (IsKeyPressed(KEY_T) && !multiSelect && underTile) 
    {
        // Pick the (T)extures from the tile under the cursor.
        for (int i = 0; i < TEXTURES_PER_TILE; ++i) 
        {
            fs::path path = mapMan.PathFromTexID(underTile.textures[i]);
            textures[i] = Assets::GetTexture(path);
        }
    }
}

PlaceMode::TileCursor::Draw()
{
    if (!IsKeyDown(KEY_LEFT_SHIFT))
    {
        for (int i = 0; i < TEXTURES_PER_TILE; ++i)
        {
            materials[i].maps[MATERIAL_MAP_ALBEDO].texture = textures[i]->GetTexture();
        }
        
        Matrix cursorTransform = TileRotationMatrix(yaw, pitch)
            * MatrixTranslate(position.x, position.y, position.z));
        
        const Model &shape = _tileCursor.model->GetModel();
        for (size_t m = 0; m < shape.meshCount; ++m) 
        {
            DrawMesh(shape.meshes[m], materials[m], cursorTransform);
        }
    }
}