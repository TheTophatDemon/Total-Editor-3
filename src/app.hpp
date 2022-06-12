#ifndef APP_HPP
#define APP_HPP

#include "raylib.h"
#include <string>

//Contains settings and other "application wide" information.
struct AppContext {
    size_t undoStackSize;
    float mouseSensitivity;
    Texture2D *selectedTexture;
    Model *selectedShape;
    std::string texturesDir;
    std::string shapesDir;
};

#endif