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

#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

//A repository that caches all loaded resources and their file paths.
//It is implemented as a singleton with a static interface. 
//(This circumvents certain limitations regarding static members and allows the constructor to be called automatically when the first method is called.)
class Assets 
{
public:
    //A RAII wrapper for a Raylib Texture (unloads on destruction)
    class TexHandle 
    {
    public:
        inline TexHandle(Texture2D texture, fs::path path) { _texture = texture; _path = path; }
        inline ~TexHandle() { UnloadTexture(_texture); }
        inline Texture2D GetTexture() const { return _texture; }
        inline fs::path GetPath() const { return _path; }
    private:
        Texture2D _texture;
        fs::path _path;
    };

    // Determines which cardinal direction in which to cull certain faces when the neighboring tile is occupied.
    enum class CullGroup { NO_CULL, CULL_N, CULL_S, CULL_E, CULL_W, CULL_U, CULL_D };
    // Returns a unit vector pointing in the direction represented by the cull group.
    static Vector3 CullGroupVector(const CullGroup group);

    //A RAII wrapper for a Raylib Model (unloads on destruction)
    class ModelHandle
    {
    public:
        // Loads an .obj model from the given path and initializes a handle for it.
        ModelHandle(const fs::path path); 
        ~ModelHandle();
        inline Model GetModel() const { return _model; }
        inline fs::path GetPath() const { return _path; }
        CullGroup GetCullGroup(const Mesh* mesh);
        CullGroup GetCullGroup(const size_t meshIndex);
    private:
        Model _model;
        fs::path _path;
        CullGroup* _cullGroups; // Indexed by the mesh index of the model, returns the culling group assigned to the mesh.
    };

    static std::shared_ptr<TexHandle>   GetTexture(fs::path path); //Returns a shared pointer to the cached texture at `path`, loading it if it hasn't been loaded.
    static std::shared_ptr<ModelHandle> GetModel(fs::path path);   //Returns a shared pointer to the cached model at `path`, loading it if it hasn't been loaded.

    static const Font&   GetFont(); //Returns the default application font (dejavu.fnt)
    static const Shader& GetMapShader(bool instanced); //Returns the shader used to render tiles
    static const Model&  GetEntSphere(); //Returns the sphere that represents entities visually
protected:
    //Asset caches that hold weak references to all the loaded textures and models
    std::map<fs::path, std::weak_ptr<TexHandle>>   _textures;
    std::map<fs::path, std::weak_ptr<ModelHandle>> _models;

    //Assets that are alive the whole application
    Font _font; //Default application font (dejavu.fnt)
    Texture2D _missingTexture; //Texture to display when the texture file to be loaded isn't found
    Model _entSphere; //The sphere that represents entities visually
    Shader _mapShader; //The non-instanced version that is used to render tiles outside of the map itself
    Shader _mapShaderInstanced; //The instanced version is used to render the tiles
private:
    Assets();
    ~Assets();
    static Assets *_Get();
};

#endif
