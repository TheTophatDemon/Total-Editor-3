/**
 * Copyright (c) 2022-present Alexander Lunsford
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

#include "assets.hpp"

#include "raymath.h"
#include "rlgl.h"

#include "assets/shaders/map_shader.hpp"
#include "assets/shaders/sprite_shader.hpp"
#include "assets/fonts/font_dejavu.h"
#include "assets/obj_loader.hpp"
#include "c_helpers.hpp"

#include <iostream>
#include <assert.h>

static Assets *_instance = nullptr;

Assets *Assets::_Get() 
{
    if (!_instance)
    {
        _instance = new Assets();
    }
    return _instance;
};

Assets::ModelHandle::ModelHandle(fs::path path) 
{ 
    _path = path; 
    _model = LoadOBJModelButBetter(path);
}

Assets::ModelHandle::~ModelHandle() 
{ 
    UnloadModel(_model); 
}

Assets::Assets() 
{
    // Generate missing texture image (a black-and-magenta checkerboard)
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
                pixels[base] = char(0xFF);
                pixels[base + 1] = 0x00;
                pixels[base + 2] = char(0xFF);
            }
            else
            {
                pixels[base] = pixels[base + 1] = pixels[base + 2] = 0x00;
            }
        }
    }
    _missingTexture = LoadTextureFromImage(texImg);
    free(texImg.data);

    // Initialize instanced shader for map geometry
    _mapShaderInstanced = LoadShaderFromMemory(MAP_SHADER_INSTANCED_V_SRC, MAP_SHADER_F_SRC);
    _mapShaderInstanced.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShaderInstanced, "mvp");
    _mapShaderInstanced.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShaderInstanced, "viewPos");
    _mapShaderInstanced.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_mapShaderInstanced, "instanceTransform");

    _mapShader = LoadShaderFromMemory(MAP_SHADER_V_SRC, MAP_SHADER_F_SRC);
    _mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
    _mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");

    // Sprite shader
    _spriteShader = LoadShaderFromMemory(SPRITE_SHADER_V_SRC, SPRITE_SHADER_F_SRC);
    _mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
    _mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");
    _spriteShader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(_spriteShader, "matModel");
    _spriteShader.locs[SHADER_LOC_MATRIX_VIEW] = GetShaderLocation(_spriteShader, "matView");
    _spriteShader.locs[SHADER_LOC_MATRIX_PROJECTION] = GetShaderLocation(_spriteShader, "matProj");

    // Initialize the sprite's quad model
    {
        Mesh m = Mesh { 
            .vertexCount = 4,
            .triangleCount = 2,
            .vertices = SAFE_MALLOC(float, 4 * 3),
            .texcoords = SAFE_MALLOC(float, 4 * 2),
            .indices = SAFE_MALLOC(unsigned short, 2 * 3)
        };
        // Vertex positions (x, y, z)
        m.vertices[0] = -1.0f; m.vertices[ 1] = +1.0f; m.vertices[ 2] = 0.0f;
        m.vertices[3] = +1.0f; m.vertices[ 4] = +1.0f; m.vertices[ 5] = 0.0f;
        m.vertices[6] = +1.0f; m.vertices[ 7] = -1.0f; m.vertices[ 8] = 0.0f;
        m.vertices[9] = -1.0f; m.vertices[10] = -1.0f; m.vertices[11] = 0.0f;
        // UV
        m.texcoords[0] = 0.0f; m.texcoords[1] = 0.0f;
        m.texcoords[2] = 1.0f; m.texcoords[3] = 0.0f;
        m.texcoords[4] = 1.0f; m.texcoords[5] = 1.0f;
        m.texcoords[6] = 0.0f; m.texcoords[7] = 1.0f;
        // Indices
        m.indices[0] = 2; m.indices[1] = 1; m.indices[2] = 0;
        m.indices[3] = 3; m.indices[4] = 2; m.indices[5] = 0;
        UploadMesh(&m, false);
        _spriteQuad = m;
    }

    // Generate entity sphere
    _entSphere = LoadModelFromMesh(GenMeshSphere(1.0f, 8, 8));
    for (int m = 0; m < _entSphere.materialCount; ++m)
    {
        _entSphere.materials[m].shader = _mapShader;
    }

    _font = LoadFont_Dejavu();
}

const Font &Assets::GetFont() 
{
    return _Get()->_font;
}

const Shader &Assets::GetMapShader(bool instanced) 
{
    return instanced ? _Get()->_mapShaderInstanced : _Get()->_mapShader;
}

const Shader &Assets::GetSpriteShader()
{
    return _Get()->_spriteShader;
}

const Model &Assets::GetEntSphere()
{
    return _Get()->_entSphere;
}

const Mesh& Assets::GetSpriteQuad()
{
    return _Get()->_spriteQuad;
}

Texture Assets::GetMissingTexture()
{
    return _Get()->_missingTexture;
}

std::shared_ptr<Assets::TexHandle> Assets::GetTexture(fs::path texturePath)
{
    Assets *a = _Get();
    //Attempt to find the texture in the cache
    if (a->_textures.find(texturePath) != a->_textures.end())
    {
        std::weak_ptr<TexHandle> weakHandle = a->_textures[texturePath];
        if (std::shared_ptr<TexHandle> handle = weakHandle.lock())
        {
            return handle;
        }
    }

    //Load the texture if it is no longer stored in the cache
    Texture2D texture = LoadTexture(texturePath.string().c_str());
    //Replace with the checkerboard texture if the file didn't load
    if (texture.width == 0) texture = a->_missingTexture;

    auto sharedPtr = std::make_shared<TexHandle>(texture, texturePath);
    //Cache the texture
    a->_textures[texturePath] = std::weak_ptr<TexHandle>(sharedPtr);
    return sharedPtr;
}

std::shared_ptr<Assets::ModelHandle> Assets::GetModel(fs::path path)
{
    Assets *a = _Get();
    //Attempt to find the model in the cache
    if (a->_models.find(path) != a->_models.end())
    {
        std::weak_ptr<ModelHandle> weakHandle = a->_models[path];
        if (std::shared_ptr<ModelHandle> handle = weakHandle.lock())
        {
            return handle;
        }
    }

    //Load the model if it is no longer stored in the cache
    auto sharedPtr = std::make_shared<ModelHandle>(path);
    //Cache the model
    a->_models[path] = std::weak_ptr<ModelHandle>(sharedPtr);
    return sharedPtr;
}
