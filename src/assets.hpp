#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

#include <string>

namespace Assets {

    #define SHAPE_ICON_SIZE 64

    void Initialize();
    void Update();
    void Unload();
    Texture2D *GetTexture(const std::string texturePath);
    Material *GetMaterialForTexture(const std::string texturePath, bool instanced = false);
    Material *GetMaterialForTexture(const Texture2D *texture, bool instanced = false);
    Model *GetShape(const std::string modelPath);
    Texture2D *GetShapeIcon(const Model *shape);
    void DrawShapeIcon(const RenderTexture2D& target, const Model *shape);
    Font *GetFont();

};

#endif