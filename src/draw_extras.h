/*
Copyright (C) 2022 Alexander Lunsford
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

inline void DrawAxes3D(Vector3 position, float scale)
{
    //Draw axes
    rlBegin(RL_LINES);
    
    rlColor3f(1.0f, 0.0f, 0.0f); 
    //Line X
    rlVertex3f(position.x, position.y, position.z); rlVertex3f(position.x + scale, position.y, position.z);
    //X
    rlVertex3f(position.x + scale * 1.0f, position.y + scale * 0.25f, position.z); rlVertex3f(position.x + scale * 1.5f, position.y + scale * -0.25f, position.z); 
    rlVertex3f(position.x + scale * 1.5f, position.y + scale * 0.25f, position.z); rlVertex3f(position.x + scale * 1.0f, position.y + scale * -0.25f, position.z);
    
    rlColor3f(0.0f, 1.0f, 0.0f); 
    //Line Y
    rlVertex3f(position.x, position.y, position.z); rlVertex3f(position.x, position.y + scale, position.z);
    //Y
    rlVertex3f(position.x, position.y + scale * 1.25f,position.z); rlVertex3f(position.x + scale * 0.0f,  position.y + scale * 1.5f,  position.z);
    rlVertex3f(position.x, position.y + scale * 1.5f, position.z); rlVertex3f(position.x + scale * 0.25f, position.y + scale *  1.75f,position.z);
    rlVertex3f(position.x, position.y + scale * 1.5f, position.z); rlVertex3f(position.x + scale *-0.25f, position.y + scale *  1.75f,position.z);

    rlColor3f(0.0f, 0.0f, 1.0f); 
    //Line Z
    rlVertex3f(position.x, position.y, position.z); rlVertex3f(position.x, position.y, position.z + scale);
    //Z
    rlVertex3f(position.x, position.y + scale * -0.25f,position.z +  scale * 1.25f); rlVertex3f(position.x, position.y + scale * -0.25f,position.z +  scale * 1.75f);
    rlVertex3f(position.x, position.y + scale * -0.25f,position.z +  scale * 1.75f); rlVertex3f(position.x, position.y + scale * 0.25f, position.z + scale * 1.25f);
    rlVertex3f(position.x, position.y + scale * 0.25f, position.z + scale * 1.25f); rlVertex3f( position.x, position.y + scale * 0.25f, position.z + scale * 1.75f);

    rlSetLineWidth(2.0f);
    rlEnd();
}

#endif