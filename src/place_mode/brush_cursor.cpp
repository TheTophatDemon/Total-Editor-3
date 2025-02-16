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

PlaceMode::BrushCursor::BrushCursor(MapMan& mapMan)
    : brush(mapMan, 1, 1, 1)
{}

void PlaceMode::BrushCursor::Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l)
{
    // Put the end position at the other extent of the bounding box so that a border can be drawn later
    endPosition = Vector3 {
        position.x + ((brush.GetWidth() - 1) * brush.GetSpacing()),
        position.y + ((brush.GetHeight() - 1) * brush.GetSpacing()),
        position.z + ((brush.GetLength() - 1) * brush.GetSpacing()),
    };

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_ALT))
    {
        mapMan.ExecuteTileAction(i, j, k, w, h, l, brush);
    }
}

void PlaceMode::BrushCursor::Draw()
{
    // Draw the tile grid within the brush
    Vector3 brushOffset = Vector3Min(position, endPosition);
    brushOffset.x -= brush.GetSpacing() / 2.0f;
    brushOffset.y -= brush.GetSpacing() / 2.0f;
    brushOffset.z -= brush.GetSpacing() / 2.0f;
    brush.Draw(brushOffset);
}