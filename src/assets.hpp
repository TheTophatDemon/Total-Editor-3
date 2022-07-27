/**
 * Copyright (c) 2022 Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

#include <string>
#include <vector>
#include <map>
#include <filesystem>
namespace fs = std::filesystem;

typedef int ModelID;
typedef int TexID;

#define NO_TEX -1
#define NO_MODEL -1

//A repository that caches all loaded resources and their file paths, indexing some using integer IDs.
//It is implemented as a singleton with a static interface.
class Assets 
{
public:
    static TexID TexIDFromPath(fs::path texturePath);
    static fs::path PathFromTexID(TexID texID);
    static const Texture2D &TexFromID(TexID texID);
    static const Material &GetMaterialForTexture(TexID texID, bool instanced);
    static TexID FindLoadedMaterialTexID(const Material &material, bool instanced);
    static ModelID ModelIDFromPath(fs::path modelPath);
    static fs::path PathFromModelID(ModelID modelID);
    static const Model &ModelFromID(ModelID modelID);
    
    static const Texture2D &GetShapeIcon(ModelID shape);

    static const Font &GetFont();
    static const Shader &GetMapShader(bool instanced = true);
    static const Model &GetEntSphere();

    //Loads new textures from the fileList, in order of increasing texID.
    static void LoadTextureIDs(const std::vector<fs::path> &fileList);
    //Loads new models from the fileList, in order of increasing modelID.
    static void LoadShapeIDs(const std::vector<fs::path> &fileList);

    static void RedrawIcons();

    //Releases all memory and ID associations.
    static void Clear();
protected:
    std::map<TexID, std::pair<fs::path, Texture2D>>  _textures;
    std::map<TexID, Material>                        _materials; //Materials that use the default shader.
    std::map<TexID, Material>                        _instancedMaterials; //Materials that use the instanced shader.
    std::map<ModelID, std::pair<fs::path, Model>>    _models;
    std::map<ModelID, RenderTexture2D>               _shapeIcons;
    Shader _mapShaderInstanced; //Instanced shader for drawing map geometry
    Shader _mapShader; //Non-instanced shader for drawing map geometry.
    Font _font; //Default application font (dejavu.fnt)
    Texture2D _missingTexture;
    Model _missingModel;
    Model _entSphere;
    TexID _nextTexID;
    ModelID _nextModelID;
private:
    Assets();
    ~Assets();
    static Assets *_Get();
};

#endif
