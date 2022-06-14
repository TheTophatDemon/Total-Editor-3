#ifndef GRID_EXTRAS_H
#define GRID_EXTRAS_H

#include "rlgl.h"

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

//Draw a grid centered at the given Vector3 `position`, with a rectangular size given by `slicesX` and `slicesZ`.
void DrawGridEx(Vector3 position, int slicesX, int slicesZ, float spacing)
{
    int halfSlicesX = slicesX/2;
    int halfSlicesZ = slicesZ/2;

    rlCheckRenderBatchLimit((slicesX + slicesZ + 4)*2);

    rlBegin(RL_LINES);
        for (int i = -halfSlicesX; i <= halfSlicesX; i++)
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

            rlVertex3f(position.x + (float)i*spacing, position.y, position.z + (float)-halfSlicesZ*spacing);
            rlVertex3f(position.x + (float)i*spacing, position.y, position.z + (float)halfSlicesZ*spacing);
        }

        for (int j = -halfSlicesZ; j <= halfSlicesZ; j++) 
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

            rlVertex3f(position.x + (float)-halfSlicesX*spacing, position.y, position.z + (float)j*spacing);
            rlVertex3f(position.x + (float)halfSlicesX*spacing, position.y, position.z + (float)j*spacing);
        }
    rlEnd();
}

void DrawAxes3D(Vector3 position, float scale)
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

#if defined(__cplusplus)
}          
#endif

#endif