#include "assets.hpp"

#include <unordered_map>

static std::unordered_map<std::string, Material> _normalMaterials;
static std::unordered_map<std::string, Material> _instancedMaterials;
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

Material *Assets::GetMaterialForTexture(const std::string& texturePath, bool instanced) {
    if (_textures.find(texturePath) == _textures.end()) {
        _textures[texturePath] = LoadTexture(texturePath.c_str());
    }
    if (instanced) {
        if (_instancedMaterials.find(texturePath) == _instancedMaterials.end()) {
            Material mat = LoadMaterialDefault();
            SetMaterialTexture(&mat, MATERIAL_MAP_ALBEDO, _textures[texturePath]);
            mat.shader = _mapShader;
            _instancedMaterials[texturePath] = mat; 
        }
        return &_instancedMaterials[texturePath];
    } else {
        if (_normalMaterials.find(texturePath) == _normalMaterials.end()) {
            Material mat = LoadMaterialDefault();
            SetMaterialTexture(&mat, MATERIAL_MAP_ALBEDO, _textures[texturePath]);
            _normalMaterials[texturePath] = mat; 
        }
        return &_normalMaterials[texturePath];
    }
}

Model *Assets::GetShape(const std::string& modelPath) {
    if (_shapes.find(modelPath) == _shapes.end()) {
        _shapes[modelPath] = LoadModel(modelPath.c_str());
    }
    return &_shapes[modelPath];
}

Font *Assets::GetFont() {
    return &_font;
}