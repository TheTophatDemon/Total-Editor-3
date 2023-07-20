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

Assets::ModelHandle::ModelHandle(fs::path path) 
{ 
    _path = path; 

    // We are loading the .OBJ file manually because Raylib's loader doesn't take indices into account.
    typedef struct Vertex 
    {
        Vector3 pos;
        Vector2 uv;
        Vector3 norm;
    } Vertex;

    std::vector<Vertex> meshVerts;
    std::vector<unsigned short> meshInds;
    // This takes a triplet string from the .obj face and points it to a vertex in meshVertices
    std::map<std::string, int> vertMapper;

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

    while (std::getline(objFile, line), !line.empty())
    {
        // Get the items between spaces
        std::vector<std::string> tokens = SplitString(line, " ");
        if (tokens.empty()) continue;

        if (tokens[0].compare("v") == 0) // Vertex positions
        {
            objPositions.push_back(
                Vector3 {
                    (float)atof(tokens[1].c_str()),
                    (float)atof(tokens[2].c_str()),
                    (float)atof(tokens[3].c_str())
                });
        }
        else if (tokens[0].compare("vt") == 0) // Vertex texture coordinates
        {
            objUVs.push_back(
                Vector2 {
                    (float)atof(tokens[1].c_str()),
                    1.0f - (float)atof(tokens[2].c_str())
                });
        }
        else if (tokens[0].compare("vn") == 0) // Vertex normals
        {
            objNormals.push_back(
                Vector3 {
                    (float)atof(tokens[1].c_str()),
                    (float)atof(tokens[2].c_str()),
                    (float)atof(tokens[3].c_str())
                });
        }
        else if (tokens[0].compare("f") == 0) // Faces
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

                // Add indices to mesh
                if (vertMapper.find(tokens[i]) != vertMapper.end())
                {
                    meshInds.push_back(vertMapper[tokens[i]]);
                }
                else
                {
                    vertMapper[tokens[i]] = meshVerts.size();
                    meshInds.push_back(meshVerts.size());
                    meshVerts.push_back(
                        Vertex { 
                            objPositions[posIdx], 
                            objUVs[uvIdx], 
                            objNormals[normIdx]
                        });
                }
            }
        }
        else if (tokens[0].compare("o") == 0) // Objects
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
    _model.materialCount = 1;
    _model.materials = (Material*) RL_MALLOC(sizeof(Material) * _model.materialCount);
    _model.meshCount = _model.materialCount;
    _model.meshes = (Mesh*) RL_MALLOC(sizeof(Mesh) * _model.meshCount);
    _model.meshMaterial = (int*) RL_MALLOC(sizeof(Mesh) * _model.meshCount);
    _model.transform = MatrixIdentity();

    int m = 0;
    _model.materials[m] = LoadMaterialDefault();
    _model.meshMaterial[m] = m;

    // Encode the dynamic vertex arrays into a Raylib mesh
    Mesh mesh = { 0 };
    mesh.vertices = (float*) malloc(meshVerts.size() * 3 * sizeof(float));
    mesh.vertexCount = meshVerts.size();
    mesh.texcoords = (float*) malloc(meshVerts.size() * 2 * sizeof(float));
    mesh.texcoords2 = mesh.animNormals = mesh.animVertices = mesh.boneWeights = mesh.tangents = nullptr;
    mesh.boneIds = mesh.colors = nullptr;
    mesh.normals = (float*) malloc(meshVerts.size() * 3 * sizeof(float));
    mesh.indices = (unsigned short*) malloc(meshInds.size() * sizeof(unsigned short));
    mesh.triangleCount = meshInds.size() / 3;
    mesh.vaoId = 0, mesh.vboId = 0;

    for (int v = 0, p = 0, n = 0, u = 0; v < meshVerts.size(); ++v)
    {
        mesh.vertices[p++] = meshVerts[v].pos.x;
        mesh.vertices[p++] = meshVerts[v].pos.y;
        mesh.vertices[p++] = meshVerts[v].pos.z;
        mesh.normals[n++] = meshVerts[v].norm.x;
        mesh.normals[n++] = meshVerts[v].norm.y;
        mesh.normals[n++] = meshVerts[v].norm.z;
        mesh.texcoords[u++] = meshVerts[v].uv.x;
        mesh.texcoords[u++] = meshVerts[v].uv.y;
    }
    for (int i = 0; i < meshInds.size(); ++i)
    {
        mesh.indices[i] = meshInds[i];
    }
    UploadMesh(&mesh, false);
    
    _model.meshes[m] = mesh;
}

Assets::ModelHandle::~ModelHandle() 
{ 
    UnloadModel(_model); 
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

    //Initialize instanced shader for map geometry
    _mapShaderInstanced = LoadShaderFromMemory(MAP_SHADER_INSTANCED_V_SRC, MAP_SHADER_F_SRC);
    _mapShaderInstanced.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShaderInstanced, "mvp");
    _mapShaderInstanced.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShaderInstanced, "viewPos");
    _mapShaderInstanced.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocationAttrib(_mapShaderInstanced, "instanceTransform");

    _mapShader = LoadShaderFromMemory(MAP_SHADER_V_SRC, MAP_SHADER_F_SRC);
    _mapShader.locs[SHADER_LOC_MATRIX_MVP] = GetShaderLocation(_mapShader, "mvp");
    _mapShader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(_mapShader, "viewPos");

    //Generate entity sphere
    _entSphere = LoadModelFromMesh(GenMeshSphere(1.0f, 8, 8));
    for (int m = 0; m < _entSphere.materialCount; ++m)
        _entSphere.materials[m].shader = _mapShader;

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
