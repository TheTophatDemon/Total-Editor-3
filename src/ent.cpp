#include "ent.hpp"

#include "raymath.h"
#include "rlgl.h"

#include "draw_extras.h"

void EntGrid::Draw(Camera &camera, int fromY, int toY)
{
    float lowerY = (float) fromY * _spacing - (_spacing / 2.0f);
    float upperY = (float) toY * _spacing + (_spacing / 2.0f);
    for (const Ent &ent : _ents)
    {
        if (ent.position.y > lowerY && ent.position.y < upperY) 
        {
            ent.Draw(camera);
        }
    }
}

void Ent::Draw(Camera &camera) const
{
    if (model != nullptr)
    {
        //Draw model
        Quaternion quat = QuaternionFromEuler(ToRadians((float) pitch), ToRadians((float) yaw), 0.0f);
        Vector3 axis = (Vector3) { 0 };
        float angle = 0.0f;
        QuaternionToAxisAngle(quat, &axis, &angle);
        DrawModelEx(*model, position, axis, angle, Vector3One(), WHITE);
    }
    else 
    {
        if (sprite != nullptr)
        {
            //Draw billboard
            DrawBillboard(camera, *sprite, position, 2.0f, WHITE);
        }

        //Draw axes to show orientation
        rlPushMatrix();
        rlMultMatrixf(MatrixToFloat(GetMatrix()));
        DrawAxes3D(position, 1.0f);
        rlPopMatrix();
    }
}