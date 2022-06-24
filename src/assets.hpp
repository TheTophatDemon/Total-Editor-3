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
    static const Shader &GetMapShader();

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
    Shader _mapShader; //Instanced shader for drawing map geometry
    Font _font; //Default application font (dejavu.fnt)
    Texture2D _missingTexture;
    Model _missingModel;
    TexID _nextTexID;
    ModelID _nextModelID;
private:
    Assets();
    ~Assets();
    static Assets *_Get();
};

#endif