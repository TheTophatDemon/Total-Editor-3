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

#ifndef GRID_EXTRAS_H
#define GRID_EXTRAS_H

#include "rlgl.h"

//Draw a grid centered at the given Vector3 `position`, with a rectangular size given by `slicesX` and `slicesZ`.
inline void DrawGridEx(Vector3 position, int slicesX, int slicesZ, float spacing)
{
    const float RADIUS_X = spacing * (float)(slicesX - 1) / 2.0f;
    const float RADIUS_Z = spacing * (float)(slicesZ - 1) / 2.0f;

    rlCheckRenderBatchLimit((slicesX + slicesZ + 4)*2);

    rlBegin(RL_LINES);
        for (int i = 0; i < slicesX; i++)
        {
            if (i == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f(position.x - RADIUS_X + (float)i*spacing, position.y, position.z - RADIUS_Z);
            rlVertex3f(position.x - RADIUS_X + (float)i*spacing, position.y, position.z + RADIUS_Z);
        }

        for (int j = 0; j < slicesZ; j++) 
        {
            if (j == 0)
            {
                rlColor3f(0.5f, 0.5f, 0.5f);
                rlColor3f(0.5f, 0.5f, 0.5f);
            }
            else
            {
                rlColor3f(0.75f, 0.75f, 0.75f);
                rlColor3f(0.75f, 0.75f, 0.75f);
            }

            rlVertex3f(position.x - RADIUS_X, position.y, position.z - RADIUS_Z + (float)j*spacing);
            rlVertex3f(position.x + RADIUS_X, position.y, position.z - RADIUS_Z + (float)j*spacing);
        }
    rlEnd();
}

inline void DrawAxes3D(Vector3 position, Vector3 scale)
{
    float letterScale = Minf(scale.x, Minf(scale.y, scale.z));

    //Draw axes
    rlBegin(RL_LINES);
    
    rlColor3f(1.0f, 0.0f, 0.0f); 
    //Line X
    rlVertex3f(position.x, position.y, position.z); rlVertex3f(position.x + (scale.x * 1.0f) - 0.25f, position.y, position.z);
    //X
    float xOffset = position.x + (scale.x * 1.0f);
    rlVertex3f(xOffset, position.y + (letterScale * 0.25f), position.z); rlVertex3f(xOffset + (letterScale * 0.5f), position.y + (letterScale * -0.25f), position.z); 
    rlVertex3f(xOffset + (letterScale * 0.5f), position.y + (letterScale * 0.25f), position.z); rlVertex3f(xOffset, position.y + (letterScale * -0.25f), position.z);
    
    rlColor3f(0.0f, 1.0f, 0.0f); 
    //Line Y
    rlVertex3f(position.x, position.y, position.z); rlVertex3f(position.x, position.y + (scale.y * 1.25f) - 0.25f, position.z);
    //Y
    float yOffset = position.y + (scale.y * 1.25f);
    rlVertex3f(position.x, yOffset, position.z); rlVertex3f(position.x + letterScale * 0.0f, yOffset + (letterScale * 0.25), position.z);
    rlVertex3f(position.x, yOffset + (letterScale * 0.25f), position.z); rlVertex3f(position.x + letterScale * 0.25f, yOffset + (letterScale * 0.5), position.z);
    rlVertex3f(position.x, yOffset + (letterScale * 0.25f), position.z); rlVertex3f(position.x + letterScale * -0.25f, yOffset + (letterScale * 0.5), position.z);

    rlColor3f(0.0f, 0.0f, 1.0f); 
    //Line Z
    rlVertex3f(position.x, position.y, position.z); rlVertex3f(position.x, position.y, position.z + (scale.z * 1.25f) - 0.25f);
    //Z
    float zOffset = position.z + (scale.z * 1.25f);
    rlVertex3f(position.x, position.y + (letterScale * -0.25f), zOffset); rlVertex3f(position.x, position.y + (letterScale * -0.25f), zOffset + (letterScale * 0.5f));
    rlVertex3f(position.x, position.y + (letterScale * -0.25f), zOffset + (letterScale * 0.5f)); rlVertex3f(position.x, position.y + (letterScale * 0.25f), zOffset);
    rlVertex3f(position.x, position.y + (letterScale * 0.25f), zOffset); rlVertex3f(position.x, position.y + (letterScale * 0.25f), zOffset + (letterScale * 0.5f));

    rlSetLineWidth(2.0f);
    rlEnd();
}

#endif
