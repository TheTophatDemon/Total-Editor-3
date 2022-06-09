#ifndef APP_HPP
#define APP_HPP

#include "raylib.h"

//Contains settings and other "application wide" information.
struct AppContext {
    int screenWidth;
    int screenHeight;
    float mouseSensitivity;
    Texture2D *selectedTexture;
    Model *selectedShape;
};

#endif