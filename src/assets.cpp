#include "assets.hpp"

#include <unordered_map>

static std::unordered_map<const Texture *, Material> _normalMaterials;
static std::unordered_map<const Texture *, Material> _instancedMaterials;
static std::unordered_map<std::string, Texture2D> _textures;
static Shader _mapShader;

static std::unordered_map<std::string, Model> _shapes;

static Font _font;

void Assets::Initialize() {
    //Initialize instanced shader for map geometry
    _mapShader = LoadShader("assets/shaders/map_geom.vs", "assets/shaders/map_geom.fs");
    _mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
    _mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");
    _mapShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_mapShader, "instanceTransform");

    _font = LoadFont("assets/fonts/dejavu.fnt");
}

Texture *Assets::GetTexture(const std::string texturePath) {
    if (_textures.find(texturePath) == _textures.end()) {
        _textures[texturePath] = LoadTexture(texturePath.c_str());
    }
    return &_textures[texturePath];
}

Material *Assets::GetMaterialForTexture(const std::string texturePath, bool instanced) {
    return GetMaterialForTexture(GetTexture(texturePath), instanced);
}

Material *Assets::GetMaterialForTexture(const Texture2D *texture, bool instanced) {
    if (instanced) {
        if (_instancedMaterials.find(texture) == _instancedMaterials.end()) {
            Material mat = LoadMaterialDefault();
            SetMaterialTexture(&mat, MATERIAL_MAP_ALBEDO, *texture);
            mat.shader = _mapShader;
            _instancedMaterials[texture] = mat; 
        }
        return &_instancedMaterials[texture];
    } else {
        if (_normalMaterials.find(texture) == _normalMaterials.end()) {
            Material mat = LoadMaterialDefault();
            SetMaterialTexture(&mat, MATERIAL_MAP_ALBEDO, *texture);
            _normalMaterials[texture] = mat; 
        }
        return &_normalMaterials[texture];
    }
}

Model *Assets::GetShape(const std::string modelPath) {
    if (_shapes.find(modelPath) == _shapes.end()) {
        _shapes[modelPath] = LoadModel(modelPath.c_str());
    }
    return &_shapes[modelPath];
}

Font *Assets::GetFont() {
    return &_font;
}