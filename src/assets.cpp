#include "assets.hpp"

#include "raymath.h"
#include "rlgl.h"

#include "assets/shaders/map_shader.hpp"
#include "assets/fonts/font_dejavu.h"

#include <iostream>
#include <unordered_map>
#include <fstream>

#define SHAPE_ICON_SIZE 64

static Assets *_instance = nullptr;

Assets *Assets::_Get() {
    if (!_instance)
    {
        _instance = new Assets();
    }
    return _instance;
};

Assets::Assets() 
{
    //Generate missing texture image
    Image texImg = { 0 };
    texImg.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;
    texImg.width = 64;
    texImg.height = 64;
    texImg.mipmaps = 1;
    char *pixels = (char *) malloc(3 * texImg.width * texImg.height);
    texImg.data = (void *) pixels;
    const int BLOCK_SIZE = 32;
    for (int x = 0; x < texImg.width; ++x)
    {
        for (int y = 0; y < texImg.height; ++y)
        {
            int base = 3 * (x + y * texImg.width);
            if ((((x / BLOCK_SIZE) % 2) == 0 && ((y / BLOCK_SIZE) % 2) == 0) ||
                (((x / BLOCK_SIZE) % 2) == 1 && ((y / BLOCK_SIZE) % 2) == 1) )
            {
                pixels[base] = 0xFF;
                pixels[base + 1] = 0x00;
                pixels[base + 2] = 0xFF;
            }
            else
            {
                pixels[base] = pixels[base + 1] = pixels[base + 2] = 0x00;
            }
        }
    }
    _missingTexture = LoadTextureFromImage(texImg);
    free(texImg.data);
    
    //Assign missing model as a cube
    _missingModel = LoadModelFromMesh(GenMeshCube(2.0f, 2.0f, 2.0f));

    _entSphere = LoadModelFromMesh(GenMeshSphere(1.0f, 8, 8));

    //Initialize instanced shader for map geometry
    _mapShader = LoadShaderFromMemory(MAP_SHADER_V_SRC, MAP_SHADER_F_SRC);
    _mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
    _mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");
    _mapShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_mapShader, "instanceTransform");

    _font = LoadFont_Dejavu();

    _nextModelID = 0;
    _nextTexID = 0;
}

TexID Assets::TexIDFromPath(fs::path texturePath) 
{
    Assets *a = _Get();
    for (const auto &[id, pair] : a->_textures)
    {
        if (pair.first == texturePath) return id;
    }
    TexID id = a->_nextTexID;
    a->_textures[id] = std::pair(texturePath, LoadTexture(texturePath.c_str()));
    if (a->_textures[id].second.width == 0) a->_textures[id] = std::pair(texturePath, a->_missingTexture);
    ++a->_nextTexID;
    return id;
}

const Texture &Assets::TexFromID(TexID texID)
{
    Assets *a = _Get();
    if (texID != NO_TEX && a->_textures.find(texID) != a->_textures.end())
    {
        return a->_textures[texID].second;
    }
    else
    {
        return a->_missingTexture;
    }
}

const Material &Assets::GetMaterialForTexture(TexID texID, bool instanced) 
{
    Assets *a = _Get();
    auto &map = instanced ? a->_instancedMaterials : a->_materials;

    auto matIter = map.find(texID);
    if (matIter == map.end()) 
    {
        Material mat = LoadMaterialDefault();
        SetMaterialTexture(&mat, MATERIAL_MAP_ALBEDO, a->_textures[texID].second);
        if (instanced) mat.shader = a->_mapShader;
        map[texID] = mat; 
        return map[texID];
    }
    else
    {
        return matIter->second;
    }
}

ModelID Assets::ModelIDFromPath(fs::path modelPath) 
{
    Assets *a = _Get();
    for (const auto &[id, pair] : a->_models)
    {
        if (pair.first == modelPath) return id;
    }
    ModelID id = a->_nextModelID;
    a->_models[id] = std::pair(modelPath, LoadModel(modelPath.c_str()));
    ++a->_nextModelID;
    return id;
}

const Model &Assets::ModelFromID(ModelID modelID)
{
    Assets *a = _Get();
    if (a->_models.find(modelID) != a->_models.end())
    {
        return a->_models[modelID].second;
    }
    else
    {
        return a->_missingModel;
    }
}

const Texture2D &Assets::GetShapeIcon(ModelID modelID) 
{
    Assets *a = _Get();
    if (a->_shapeIcons.find(modelID) == a->_shapeIcons.end()) 
    {
        //Generate icon by rendering the shape onto a texture.
        a->_shapeIcons[modelID] = LoadRenderTexture(SHAPE_ICON_SIZE, SHAPE_ICON_SIZE);
        //Icon will be drawn to later.
    }

    return a->_shapeIcons[modelID].texture;
}

void Assets::RedrawIcons()
{
    static Camera camera = (Camera) {
        .position = (Vector3) { 4.0f, 4.0f, 4.0f },
        .target = Vector3Zero(),
        .up = (Vector3) { 0.0f, -1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    for (const auto &[modelID, target] : _Get()->_shapeIcons)
    {
        //Redraw the contents, because it is animated.
        BeginTextureMode(target);
        ClearBackground(BLACK);
        BeginMode3D(camera);

        DrawModelWiresEx(_Get()->ModelFromID(modelID), Vector3Zero(), (Vector3){0.0f, 1.0f, 0.0f}, GetTime() * 180.0f, Vector3One(), GREEN);

        EndMode3D();
        EndTextureMode();
    }
}

const Font &Assets::GetFont() 
{
    return _Get()->_font;
}

const Shader &Assets::GetMapShader() 
{
    return _Get()->_mapShader;
}

const Model &Assets::GetEntSphere()
{
    return _Get()->_entSphere;
}

template<typename D>
static std::vector<fs::path> GetAssetList(const std::map<int, std::pair<fs::path, D>> &map) 
{
    std::vector<fs::path> out;
    for (const auto &[key, val] : map)
    {
        out.push_back(val.first);
    }
    return out;
}

std::vector<fs::path> Assets::GetTexturePathList()
{
    return GetAssetList(_Get()->_textures);
}

std::vector<fs::path> Assets::GetShapePathList()
{
    return GetAssetList(_Get()->_models);
}

void Assets::LoadTextureIDs(const std::vector<fs::path> &fileList)
{
    for (const fs::path &path : fileList)
    {
        TexIDFromPath(path);
    }
}

void Assets::LoadShapeIDs(const std::vector<fs::path> &fileList)
{
    for (const fs::path &path : fileList)
    {
        ModelIDFromPath(path);
    }
}   

void Assets::Clear()
{
    Assets *a = _Get();
    for (const auto &[id, pair] : a->_textures)
    {
        UnloadTexture(pair.second);
    }
    a->_textures.clear();

    for (const auto &[id, pair] : a->_models)
    {
        UnloadModel(pair.second);
    }
    a->_models.clear();

    for (const auto &[id, mat] : a->_materials)
    {
        //Because the materials share textures that we have already freed, we cannot call UnloadMaterial()
        RL_FREE(mat.maps);
    }
    a->_materials.clear();

    for (const auto &[id, mat] : a->_instancedMaterials)
    {
        //Because the materials share textures that we have already freed, we cannot call UnloadMaterial()
        RL_FREE(mat.maps);
    }
    a->_instancedMaterials.clear();

    for (const auto &[id, target] : a->_shapeIcons)
    {
        UnloadRenderTexture(target);
    }
    a->_shapeIcons.clear();

    a->_nextModelID = 0;
    a->_nextTexID = 0;
}