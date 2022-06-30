/*
Copyright (C) 2022 Alexander Lunsford
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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
    static const Texture2D &TexFromID(TexID texID);
    static const Material &GetMaterialForTexture(TexID texID, bool instanced);
    static ModelID ModelIDFromPath(fs::path modelPath);
    static const Model &ModelFromID(ModelID modelID);
    
    static const Texture2D &GetShapeIcon(ModelID shape);

    static const Font &GetFont();
    static const Shader &GetMapShader(bool instanced = true);
    static const Model &GetEntSphere();

    static std::vector<fs::path> GetTexturePathList();
    static std::vector<fs::path> GetShapePathList();

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