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
#include "assets/fonts/font_dejavu.h"
#include "text_util.hpp"

#include <iostream>
#include <unordered_map>
#include <fstream>
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

Vector3 Assets::CullGroupVector(const Assets::CullGroup group)
{
    switch (group)
    {
    case CullGroup::CULL_N: return Vector3 { 0.0f,  0.0f, +1.0f };
    case CullGroup::CULL_S: return Vector3 { 0.0f,  0.0f, -1.0f };
    case CullGroup::CULL_E: return Vector3 {-1.0f,  0.0f,  0.0f };
    case CullGroup::CULL_W: return Vector3 {+1.0f,  0.0f,  0.0f };
    case CullGroup::CULL_U: return Vector3 { 0.0f, +1.0f,  0.0f };
    case CullGroup::CULL_D: return Vector3 { 0.0f, -1.0f,  0.0f };
    default: return Vector3 { 0.0f, 0.0f, 0.0f };
    }
}

Assets::ModelHandle::ModelHandle(fs::path path) 
{ 
    _path = path; 

    typedef struct Vertex 
    {
        Vector3 pos;
        Vector2 uv;
        Vector3 norm;
    } Vertex;

    typedef struct
    {
        std::vector<Vertex> verts;
        std::vector<unsigned short> inds;
        // This takes a triplet string from the .obj face and points it to a vertex in meshVertices
        std::map<std::string, int> mapper;
    } MeshBuild;

    // Maps the .obj group name to its mesh once it's loaded
    std::map<std::string, MeshBuild> meshBuilds;

    auto completeMesh = [&](const std::string& groupName, const MeshBuild& builder)->Mesh
    {
        // Encode the dynamic vertex arrays into a Raylib mesh
        Mesh mesh = { 0 };
        mesh.vertices = (float*) malloc(builder.verts.size() * 3 * sizeof(float));
        mesh.vertexCount = builder.verts.size();
        mesh.texcoords = (float*) malloc(builder.verts.size() * 2 * sizeof(float));
        mesh.texcoords2 = mesh.animNormals = mesh.animVertices = mesh.boneWeights = mesh.tangents = nullptr;
        mesh.boneIds = mesh.colors = nullptr;
        mesh.normals = (float*) malloc(builder.verts.size() * 3 * sizeof(float));
        mesh.indices = (unsigned short*) malloc(builder.inds.size() * sizeof(unsigned short));
        mesh.triangleCount = builder.inds.size() / 3;
        mesh.vaoId = 0, mesh.vboId = 0;

        for (int v = 0, p = 0, n = 0, u = 0; v < builder.verts.size(); ++v)
        {
            mesh.vertices[p++] = builder.verts[v].pos.x;
            mesh.vertices[p++] = builder.verts[v].pos.y;
            mesh.vertices[p++] = builder.verts[v].pos.z;
            mesh.normals[n++] = builder.verts[v].norm.x;
            mesh.normals[n++] = builder.verts[v].norm.y;
            mesh.normals[n++] = builder.verts[v].norm.z;
            mesh.texcoords[u++] = builder.verts[v].uv.x;
            mesh.texcoords[u++] = builder.verts[v].uv.y;
        }
        for (int i = 0; i < builder.inds.size(); ++i)
        {
            mesh.indices[i] = builder.inds[i];
        }
        UploadMesh(&mesh, false);

        return mesh;
    };

    // These track the vertex data as laid out in the .obj file
    // (Since attributes are reused between vertices, they don't map directly onto the mesh's attributes)
    std::vector<Vector3> objPositions;
    objPositions.reserve(128);
    std::vector<Vector2> objUVs;
    objUVs.reserve(128);
    std::vector<Vector3> objNormals;
    objNormals.reserve(128);

    // Parse the .obj file
    std::ifstream objFile(path);
    std::string line;
    std::string objectName;
    std::string currentGroup;

    while (std::getline(objFile, line), !line.empty())
    {
        std::vector<std::string> tokens = SplitString(line, " ");
        if (tokens.empty()) continue;
        if (tokens[0].compare("v") == 0)
        {
            objPositions.push_back(
                Vector3 {
                    (float)atof(tokens[1].c_str()),
                    (float)atof(tokens[2].c_str()),
                    (float)atof(tokens[3].c_str())
                });
        }
        else if (tokens[0].compare("vt") == 0)
        {
            objUVs.push_back(
                Vector2 {
                    (float)atof(tokens[1].c_str()),
                    1.0f - (float)atof(tokens[2].c_str())
                });
        }
        else if (tokens[0].compare("vn") == 0)
        {
            objNormals.push_back(
                Vector3 {
                    (float)atof(tokens[1].c_str()),
                    (float)atof(tokens[2].c_str()),
                    (float)atof(tokens[3].c_str())
                });
        }
        else if (tokens[0].compare("f") == 0)
        {
            if (tokens.size() > 4)
                std::cerr << "Error: Shape loaded from " << path << " should be triangulated!" << std::endl;
            
            for (int i = 1; i < tokens.size(); ++i)
            {
                std::vector<std::string> indices = SplitString(tokens[i], "/");
                if (indices.size() != 3 || indices[0].empty() || indices[1].empty() || indices[2].empty())
                {
                    std::cerr << "Error: Face in .OBJ file " << path << "does not have the right number of indices.";
                    break;
                }
                
                int posIdx = atoi(indices[0].c_str()) - 1, 
                    uvIdx = atoi(indices[1].c_str()) - 1, 
                    normIdx = atoi(indices[2].c_str()) - 1;
                if (posIdx < 0 || uvIdx < 0 || normIdx < 0)
                {
                    std::cerr << "Error: Invalid indices in .OBJ file " << path << "." << std::endl;
                    break;
                }

                MeshBuild& builder = meshBuilds[currentGroup];
                if (builder.mapper.find(tokens[i]) != builder.mapper.end())
                {
                    builder.inds.push_back(builder.mapper[tokens[i]]);
                }
                else
                {
                    builder.mapper[tokens[i]] = builder.verts.size();
                    builder.inds.push_back(builder.verts.size());
                    builder.verts.push_back(
                        Vertex { 
                            objPositions[posIdx], 
                            objUVs[uvIdx], 
                            objNormals[normIdx]
                        });
                }
            }
        }
        else if (tokens[0].compare("g") == 0)
        {
            currentGroup = tokens[1];
            
            // Initialize mesh building structure
            if (meshBuilds.find(currentGroup) == meshBuilds.end())
            {
                meshBuilds[currentGroup] = MeshBuild {
                    .verts = std::vector<Vertex>(),
                    .inds = std::vector<unsigned short>(),
                    .mapper = std::map<std::string, int>()
                };
                meshBuilds[currentGroup].verts.reserve(64);
                meshBuilds[currentGroup].inds.reserve(32);
            }
        }
        else if (tokens[0].compare("o") == 0)
        {
            if (objectName.empty())
                objectName = tokens[1];
            else
                break; // Only parse the first object in the file.
        }
    }

    // Initialize the model with the meshes
    _model = Model { 0 }; 
    _model.bindPose = nullptr;
    _model.boneCount = 0;
    _model.bones = nullptr;
    _model.materialCount = meshBuilds.size();
    _model.materials = (Material*) RL_MALLOC(sizeof(Material) * _model.materialCount);
    _model.meshCount = _model.materialCount;
    _model.meshes = (Mesh*) RL_MALLOC(sizeof(Mesh) * _model.meshCount);
    _model.meshMaterial = (int*) RL_MALLOC(sizeof(Mesh) * _model.meshCount);
    _model.transform = MatrixIdentity();

    int m = 0;
    _cullGroups = new CullGroup[_model.meshCount];
    for (const auto& [key, value] : meshBuilds)
    {
        _model.materials[m] = LoadMaterialDefault();
        _model.meshMaterial[m] = m;
        _model.meshes[m] = completeMesh(key, value);

        // Assign culling groups
        if (key.find("culln") != std::string::npos)      _cullGroups[m] = CullGroup::CULL_N;
        else if (key.find("culls") != std::string::npos) _cullGroups[m] = CullGroup::CULL_S;
        else if (key.find("culle") != std::string::npos) _cullGroups[m] = CullGroup::CULL_E;
        else if (key.find("cullw") != std::string::npos) _cullGroups[m] = CullGroup::CULL_W;
        else if (key.find("cullu") != std::string::npos) _cullGroups[m] = CullGroup::CULL_U;
        else if (key.find("culld") != std::string::npos) _cullGroups[m] = CullGroup::CULL_D;
        else                                             _cullGroups[m] = CullGroup::NO_CULL;
        ++m;
    }
}

Assets::ModelHandle::~ModelHandle() 
{ 
    delete[] _cullGroups;
    UnloadModel(_model); 
}

Assets::CullGroup Assets::ModelHandle::GetCullGroup(const Mesh* mesh)
{
    for (int i = 0; i < _model.meshCount; ++i)
    {
        if (&_model.meshes[i] == mesh)
        {
            return _cullGroups[i];
        }
    }
    std::cerr << "Error: Invalid mesh argument in ModelHandle::GetCullGroup()." << std::endl;
    return CullGroup::NO_CULL;
}

Assets::CullGroup Assets::ModelHandle::GetCullGroup(const size_t meshIndex)
{
    assert(meshIndex < _model.meshCount);
    return _cullGroups[meshIndex];
}

Assets::Assets() 
{
    //Generate missing texture image (a black-and-magenta checkerboard)
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

    //Generate entity sphere
    _entSphere = LoadModelFromMesh(GenMeshSphere(1.0f, 8, 8));

    //Initialize instanced shader for map geometry
    _mapShaderInstanced = LoadShaderFromMemory(MAP_SHADER_INSTANCED_V_SRC, MAP_SHADER_F_SRC);
    _mapShaderInstanced.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShaderInstanced, "mvp");
    _mapShaderInstanced.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShaderInstanced, "viewPos");
    _mapShaderInstanced.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_mapShaderInstanced, "instanceTransform");

    _mapShader = LoadShaderFromMemory(MAP_SHADER_V_SRC, MAP_SHADER_F_SRC);
    _mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
    _mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");

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

const Model &Assets::GetEntSphere()
{
    return _Get()->_entSphere;
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
    // Model model = LoadModel(path.string().c_str());

    auto sharedPtr = std::make_shared<ModelHandle>(path);
    //Cache the model
    a->_models[path] = std::weak_ptr<ModelHandle>(sharedPtr);
    return sharedPtr;
}
