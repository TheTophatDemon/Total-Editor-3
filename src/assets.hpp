#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

#include <string>

namespace Assets {

    void Initialize();
    Texture2D *GetTexture(const std::string texturePath);
    Material *GetMaterialForTexture(const std::string texturePath, bool instanced = false);
    Material *GetMaterialForTexture(const Texture2D *texture, bool instanced = false);
    Model *GetShape(const std::string modelPath);
    Font *GetFont();

};

#endif