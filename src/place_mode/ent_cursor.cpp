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

#include "../math_stuff.hpp"

PlaceMode::EntCursor::EntCursor() 
    : ent(1.0f)
{
    ent.properties["name"] = "entity";
}

void PlaceMode::EntCursor::Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l)
{
    disableBorder = ent.LengthsAvailable();
    bool multiSelect = ent.LengthsAvailable() && (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_LEFT));
    if (!multiSelect)
    {
        ent.lengths = Vector3{1.0f, 1.0f, 1.0f};
        endPosition = position;
    }
    else
    {
        ent.lengths = Vector3{(float)w, (float)h, (float)l};
        
        // Ensure the origin of the entity is in the tile where the mouse drag started.
        if (position.x < endPosition.x) 
        {
            ent.lengths.x *= -1;
        }
        if (position.y < endPosition.y) 
        {
            ent.lengths.y *= -1;
        }
        if (position.z < endPosition.z) 
        {
            ent.lengths.z *= -1;
        }
    }

    // Turn entity
    const int ANGLE_INC = IsKeyDown(KEY_LEFT_SHIFT) ? 15 : 45;
    if (IsKeyPressed(KEY_Q))
    {
        ent.yaw = OffsetDegrees(ent.yaw, -ANGLE_INC);
    }
    else if (!IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
    {
        ent.yaw = OffsetDegrees(ent.yaw, ANGLE_INC);
    }
    if (IsKeyPressed(KEY_F))
    {
        ent.pitch = OffsetDegrees(ent.pitch, ANGLE_INC);
    }
    else if (IsKeyPressed(KEY_V))
    {
        ent.pitch = OffsetDegrees(ent.pitch, -ANGLE_INC);
    }

    // Reset ent orientation
    if (IsKeyPressed(KEY_R))
    {
        ent.yaw = ent.pitch = 0;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_ALT) && !multiSelect)
    {
        // Placement
        if (ent.LengthsAvailable()) ent.lengths = Vector3{1.0f, 1.0f, 1.0f};
        mapMan.ExecuteEntPlacement(i, j, k, ent);
    }
    else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && multiSelect)
    {
        // Placement of box entity
        // Ensure the origin of the entity is in the tile where the mouse drag started.
        size_t i2 = i;
        size_t j2 = j;
        size_t k2 = k;
        if (position.x < endPosition.x) 
        {
            i2 += w - 1;
        }
        if (position.y < endPosition.y) 
        {
            j2 += h - 1;
        }
        if (position.z < endPosition.z) 
        {
            k2 += l - 1;
        }
        mapMan.ExecuteEntPlacement(i2, j2, k2, ent);
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        // Removal
        if (mapMan.Ents().HasEnt(i, j, k)) mapMan.ExecuteEntRemoval(i, j, k);
    }

    if (IsKeyPressed(KEY_T) || IsKeyPressed(KEY_G))
    {
        // Copy the entity under the cursor
        if (mapMan.Ents().HasEnt(i, j, k)) ent = mapMan.Ents().GetEnt(i, j, k);
    }
}

void PlaceMode::EntCursor::Draw(MapMan& mapMan)
{
    Vector3 positionToUse = position;
    if (ent.LengthsAvailable()) positionToUse = endPosition;
    ent.Draw(true, positionToUse, mapMan.Ents().GetSpacing());
}