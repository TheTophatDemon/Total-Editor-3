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

#pragma once

#ifndef MATH_STUFF_H
#define MATH_STUFF_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

const Vector3 VEC3_UP = (Vector3) { 0.0f, 1.0f, 0.0f };
const Vector3 VEC3_FORWARD = (Vector3) { 0.0f, 0.0f, -1.0f };

//Returns the parameter that is lower in value.
inline int Min(int a, int b) {
    return a < b ? a : b;
}

//Returns the parameter that is higher in value.
inline int Max(int a, int b) {
    return a < b ? b : a;
}

//Returns -1 if `a` is negative, 1 if `a` is positive, and 0 otherwise.
inline int Sign(int a) {
    if (a == 0) return 0;
    return a < 0 ? -1 : 1;
}

inline Rectangle CenteredRect(float x, float y, float w, float h)
{
    return (Rectangle) { x - (w / 2.0f), y - (h / 2.0f), w, h };
}

inline float ToRadians(float degrees)
{
    return degrees * PI / 180.0f;
}

inline float ToDegrees(float radians)
{
    return (radians / PI) * 180.0f;
}

//Ensures that the added degrees stays in the range [0, 360)
inline int OffsetDegrees(int base, int add)
{
    return (base + add >= 0) ? (base + add) % 360 : (360 + (base + add));
}

inline Matrix MatrixRotYDeg(int degrees)
{
    return MatrixRotateY(ToRadians(degrees));
}

//Modified version of GetWorldToScreen(), but returns the NDC coordinates so that the program can tell what's behind the camera.
inline Vector3 GetWorldToNDC(Vector3 position, Camera camera)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();

    // Calculate projection matrix (from perspective instead of frustum
    Matrix matProj = MatrixIdentity();

    if (camera.projection == CAMERA_PERSPECTIVE)
    {
        // Calculate projection matrix from perspective
        matProj = MatrixPerspective(camera.fovy*DEG2RAD, ((double)width/(double)height), RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }
    else if (camera.projection == CAMERA_ORTHOGRAPHIC)
    {
        float aspect = (float)width/(float)height;
        double top = camera.fovy/2.0;
        double right = top*aspect;

        // Calculate projection matrix from orthographic
        matProj = MatrixOrtho(-right, right, -top, top, RL_CULL_DISTANCE_NEAR, RL_CULL_DISTANCE_FAR);
    }

    // Calculate view matrix from camera look at (and transpose it)
    Matrix matView = MatrixLookAt(camera.position, camera.target, camera.up);

    // TODO: Why not use Vector3Transform(Vector3 v, Matrix mat)?

    // Convert world position vector to quaternion
    Quaternion worldPos = { position.x, position.y, position.z, 1.0f };

    // Transform world position to view
    worldPos = QuaternionTransform(worldPos, matView);

    // Transform result to projection (clip space position)
    worldPos = QuaternionTransform(worldPos, matProj);

    // Calculate normalized device coordinates (inverted y)
    Vector3 ndcPos = { worldPos.x/worldPos.w, -worldPos.y/worldPos.w, worldPos.z/worldPos.w };

    return ndcPos;
}

#endif