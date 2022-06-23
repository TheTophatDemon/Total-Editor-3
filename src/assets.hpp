#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

#include <string>
#include <vector>

namespace Assets {

    #define SHAPE_ICON_SIZE 64

    void Initialize();
    void Update();
    void Unload();
    Texture2D *GetTexture(const std::string texturePath);
    Material *GetMaterialForTexture(const std::string texturePath, bool instanced = false);
    Material *GetMaterialForTexture(Texture2D *texture, bool instanced = false);
    Texture2D *GetTextureForMaterial(const Material *material);
    Model *GetShape(const std::string modelPath);
    Texture2D *GetShapeIcon(const Model *shape);
    void DrawShapeIcon(const RenderTexture2D& target, const Model *shape);
    const Font &GetFont();
    Shader *GetMapShader();

    std::vector<std::string> GetTexturePathList();
    std::vector<std::string> GetShapePathList();

};

#endif