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

#include "brush_cursor.hpp"

BrushCursor::BrushCursor(MapMan& mapMan)
    : brush(mapMan, 1, 1, 1),
      originalBrush(mapMan, 1, 1, 1)

{
    disableBorder = false;
}

void BrushCursor::RotateY(MapMan& mapMan, int increments)
{
    increments %= 4;
    if (increments < 0) increments += 4;
    if (increments == 0) return;

    for (int times = 0; times < increments; ++times)
    {
        TileGrid newBrush = TileGrid(mapMan, brush.GetLength(), brush.GetHeight(), brush.GetWidth());
        for (size_t i = 0; i < newBrush.GetWidth(); ++i)
        {
            for (size_t j = 0; j < newBrush.GetHeight(); ++j)
            {
                for (size_t k = 0; k < newBrush.GetLength(); ++k)
                {
                    newBrush.SetTile(i, j, newBrush.GetLength() - 1 - k, brush.GetTile(k, j, i));
                }
            }
        }
        brush = newBrush;
    }
}

void BrushCursor::Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l)
{
    if (IsKeyPressed(KEY_Q))
    {
        RotateY(mapMan, 1);
    }
    else if (!IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_E))
    {
        RotateY(mapMan, -1);
    }
    else if (IsKeyPressed(KEY_R))
    {
        // Reset to original orientation.
        brush = originalBrush;
    }

    // Put the end position at the other extent of the bounding box so that a border can be drawn later
    endPosition = Vector3 {
        position.x + ((brush.GetWidth() - 1) * brush.GetSpacing()),
        position.y + ((brush.GetHeight() - 1) * brush.GetSpacing()),
        position.z + ((brush.GetLength() - 1) * brush.GetSpacing()),
    };

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_ALT))
    {
        mapMan.ExecuteBrushAction(i, j, k, w, h, l, brush);
    }
    else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) 
    {
        mapMan.ExecuteBrushEraseAction(i, j, k, w, h, l, brush);
    } 
}

void BrushCursor::Draw(MapMan& mapMan)
{
    // Draw the tile grid within the brush
    Vector3 brushOffset = Vector3Min(position, endPosition);
    brushOffset.x -= brush.GetSpacing() / 2.0f;
    brushOffset.y -= brush.GetSpacing() / 2.0f;
    brushOffset.z -= brush.GetSpacing() / 2.0f;
    brush.Draw(brushOffset);
}