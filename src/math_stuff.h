#ifndef MATH_STUFF_H
#define MATH_STUFF_H

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