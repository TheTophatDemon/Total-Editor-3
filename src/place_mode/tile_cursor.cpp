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

PlaceMode::TileCursor::~TileCursor()
{
    for (Material& mat : materials) 
    {
        //Free the cursor material
        //Do not use UnloadMaterial, because it will free the texture that the material uses, which might be being used somewhere else.
        RL_FREE(mat.maps);
    }
}

void PlaceMode::TileCursor::Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l)
{
    bool multiSelect = IsKeyDown(KEY_LEFT_SHIFT);
    if (!multiSelect)
    {
        endPosition = position;
    }

    Tile underTile = mapMan.Tiles().GetTile(i, j, k); // * hi. my name's sans undertile...you get it?

    // Rotate cursor
    if (IsKeyPressed(KEY_Q))
    {
        yaw = (yaw + 3) % 4;
    }
    else if (!IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
    {
        yaw = (yaw + 1) % 4;
    }
    if (IsKeyPressed(KEY_F))
    {
        pitch = (pitch + 1) % 4;
    }
    else if (IsKeyPressed(KEY_V))
    {
        pitch = (pitch + 3) % 4;
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
    else if (!multiSelect && underTile) 
    {
        // Pick the (T)extures from the tile under the cursor.
        int i = TEXTURES_PER_TILE;

        if (IsKeyPressed(KEY_T))
        {
            i = 0;
        }
        else if (IsKeyPressed(KEY_Y))
        {
            // Holding shift assigns secondary texture only.
            i = 1;
        }
        for (; i < TEXTURES_PER_TILE; ++i) 
        {
            fs::path path = mapMan.PathFromTexID(underTile.textures[i]);
            textures[i] = Assets::GetTexture(path);
        }
    }
}

void PlaceMode::TileCursor::Draw()
{
    if (!IsKeyDown(KEY_LEFT_SHIFT))
    {
        for (int i = 0; i < TEXTURES_PER_TILE; ++i)
        {
            materials[i].maps[MATERIAL_MAP_ALBEDO].texture = textures[i]->GetTexture();
        }
        
        Matrix cursorTransform = TileRotationMatrix(yaw, pitch)
            * MatrixTranslate(position.x, position.y, position.z);
        
        const Model &shape = model->GetModel();
        for (size_t m = 0; m < (size_t)shape.meshCount; ++m) 
        {
            DrawMesh(shape.meshes[m], materials[m], cursorTransform);
        }
    }
}