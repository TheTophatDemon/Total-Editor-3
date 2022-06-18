#pragma once

#ifndef MATH_STUFF_H
#define MATH_STUFF_H

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

#endif