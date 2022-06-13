#ifndef MATH_STUFF_H
#define MATH_STUFF_H

const Vector3 VEC3_UP = (Vector3) { 0.0f, 1.0f, 0.0f };
const Vector3 VEC3_FORWARD = (Vector3) { 0.0f, 0.0f, -1.0f };

//Returns the parameter that is lower in value.
int Min(int a, int b) {
    return a < b ? a : b;
}

//Returns the parameter that is higher in value.
int Max(int a, int b) {
    return a < b ? b : a;
}

//Returns -1 if `a` is negative, 1 if `a` is positive, and 0 otherwise.
int Sign(int a) {
    if (a == 0) return 0;
    return a < 0 ? -1 : 1;
}

#endif