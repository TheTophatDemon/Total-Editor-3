#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

#include <string>

namespace Assets {

    void Initialize();
    Material *GetMaterialForTexture(const std::string& texturePath, bool instanced = false);
    Model *GetShape(const std::string& modelPath);
    Font *GetFont();

};

#endif