#pragma once

#ifndef MATH_STUFF_H
#define MATH_STUFF_H

#include <tuple>
#include <math.h>
#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "json.hpp"

const Vector3 VEC3_UP = Vector3 { 0.0f, 1.0f, 0.0f };
const Vector3 VEC3_FORWARD = Vector3 { 0.0f, 0.0f, -1.0f };

// Returns the parameter that is lower in value.
inline int Min(int a, int b) 
{
    return a < b ? a : b;
}

// Returns the parameter that is lower in value.
inline float Minf(float a, float b) 
{
    return a < b ? a : b;
}

// Returns the parameter that is higher in value.
inline int Max(int a, int b) 
{
    return a < b ? b : a;
}

// Returns the parameter that is higher in value.
inline float Maxf(float a, float b) 
{
    return a < b ? b : a;
}

// Returns -1 if `a` is negative, 1 if `a` is positive, and 0 otherwise.
inline int Sign(int a) 
{
    if (a == 0) return 0;
    return a < 0 ? -1 : 1;
}

// Returns -1 if `a` is negative, 1 if `a` is positive, and 0 otherwise.
inline float Signf(float a) 
{
    if (a == 0 || isnan(a)) return 0;
    return a / fabs(a);
}

inline Rectangle CenteredRect(float x, float y, float w, float h)
{
    return Rectangle{ x - (w / 2.0f), y - (h / 2.0f), w, h };
}

inline float ToRadians(float degrees)
{
    return degrees * PI / 180.0f;
}

inline float ToDegrees(float radians)
{
    return (radians / PI) * 180.0f;
}

// Ensures that the added degrees stays in the range [0, 360)
inline int OffsetDegrees(int base, int add)
{
    return (base + add >= 0) ? (base + add) % 360 : (360 + (base + add));
}

inline Matrix MatrixRotYDeg(float degrees)
{
    return MatrixRotateY(ToRadians(degrees));
}

inline std::tuple<Vector3, float> EulerToAxisAngle(Vector3 eulerAngles) {
    Quaternion quat = QuaternionFromEuler(eulerAngles.x, eulerAngles.y, eulerAngles.z);
    Vector3 axis;
    float angle;
    QuaternionToAxisAngle(quat, &axis, &angle);
    return std::tuple<Vector3, float>{axis, angle};
}

// Modified version of Raylib's GetWorldToScreen(), but returns the NDC coordinates so that the program can tell what's behind the camera.
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

// Serialization function for BoundingBox
void to_json(nlohmann::json& j, const BoundingBox& bbox);

// Deserialization function for BoundingBox
void from_json(const nlohmann::json& j, BoundingBox& bbox);

#endif
