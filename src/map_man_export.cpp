#include "map_man.hpp"

#include "rlgl.h"
#include "json.hpp"
#include "cppcodec/base64_default_rfc4648.hpp"

#include <fstream>
#include <iostream>
#include <limits>
#include <vector>

#include "app.hpp"
#include "assets.hpp"
#include "text_util.hpp"
#include "c_helpers.hpp"

#define TARGET_ARRAY_BUFFER 34962
#define TARGET_ELEMENT_BUFFER 34963
#define COMP_TYPE_FLOAT 5126
#define COMP_TYPE_USHORT 5123
#define PRIMITIVE_MODE_TRIANGLES 4
#define FILTER_NEAREST 9728
#define FILTER_NEAREST_MIP_NEAREST 9984
#define WRAP_REPEAT 10497

bool MapMan::ExportGLTFScene(fs::path filePath, bool separateGeometry)
{
    using namespace nlohmann;

    const Model mapModel = _tileGrid.GetModel();
    
    bool isGLB = (strcmp(TextToLower(filePath.extension().string().c_str()), ".glb") == 0);
    uint8_t* bufferData = nullptr;

    bool error = false;

    try
    {   
        std::vector<json> scenes, nodes, meshes, buffers, bufferViews, accessors, materials, textures, images, samplers;

        size_t bufferOffset = 0;

        // Automates the addition of bufferViews and accessors for a given vertex attribute
        auto pushVertexAttrib = [&](size_t elemSize, size_t nElems, std::string elemType, int componentType, int target = TARGET_ARRAY_BUFFER)->size_t
        {
            // Always allocate at least one element's worth of data just to avoid errors
            size_t nBytes = Max(1, elemSize) * nElems;
            
            // Pad the offset so that the total offset of the accessor is divisible by the element size
            // This is a requirement of the gltf specifications
            size_t padding = bufferOffset % elemSize;
            bufferOffset += padding;

            size_t newIndex = bufferViews.size();

            bufferViews.push_back({
                {"buffer", 0},
                {"byteLength", nBytes},
                {"byteOffset", bufferOffset},
                {"target", target}
            });
            
            accessors.push_back({
                {"bufferView", bufferViews.size() - 1},
                {"componentType", componentType},
                {"count", nElems},
                {"type", elemType}
            });

            bufferOffset += nBytes;

            return newIndex;
        };
        
        std::vector<json> mapPrims;
        mapPrims.reserve(mapModel.meshCount);

        // Make primitives and buffer related objects for each mesh of the map
        for (int i = 0; i < mapModel.meshCount; ++i)
        {
            // Calculate max and min component values. Required only for position buffer.
            float minX, minY, minZ;
            minX = minY = minZ = std::numeric_limits<float>::max();
            float maxX, maxY, maxZ;
            maxX = maxY = maxZ = std::numeric_limits<float>::min();
            for (int j = 0; j < mapModel.meshes[i].vertexCount * 3; j += 3) 
                minX = Minf(mapModel.meshes[i].vertices[j], minX), maxX = Maxf(mapModel.meshes[i].vertices[j], maxX); 
            for (int j = 1; j < mapModel.meshes[i].vertexCount * 3; j += 3) 
                minY = Minf(mapModel.meshes[i].vertices[j], minY), maxY = Maxf(mapModel.meshes[i].vertices[j], maxY); 
            for (int j = 2; j < mapModel.meshes[i].vertexCount * 3; j += 3) 
                minZ = Minf(mapModel.meshes[i].vertices[j], minZ), maxZ = Maxf(mapModel.meshes[i].vertices[j], maxZ); 

            // Push buffers, accessors, etc.
            size_t posBufferIdx = pushVertexAttrib(sizeof(float) * 3, mapModel.meshes[i].vertexCount, "VEC3", COMP_TYPE_FLOAT);
            accessors[posBufferIdx]["min"] = {minX, minY, minZ};
            accessors[posBufferIdx]["max"] = {maxX, maxY, maxZ};

            size_t uvBufferIdx = pushVertexAttrib(sizeof(float) * 2, mapModel.meshes[i].vertexCount, "VEC2", COMP_TYPE_FLOAT);
            size_t normBufferIdx = pushVertexAttrib(sizeof(float) * 3, mapModel.meshes[i].vertexCount, "VEC3", COMP_TYPE_FLOAT);
            size_t indicesIdx = pushVertexAttrib(sizeof(unsigned short), mapModel.meshes[i].triangleCount * 3, "SCALAR", COMP_TYPE_USHORT, TARGET_ELEMENT_BUFFER);

            // Push primitive
            mapPrims.push_back({
                {"mode", PRIMITIVE_MODE_TRIANGLES},
                {"attributes", {
                    {"POSITION", posBufferIdx},
                    {"TEXCOORD_0", uvBufferIdx},
                    {"NORMAL", normBufferIdx}
                }},
                {"indices", indicesIdx},
                {"material", i}
            });
        }

        // Indices for child nodes of the root map node
        std::vector<int> mapNodeChildren;

        // Encode materials and textures
        for (int m = 0; m < mapModel.meshCount; ++m)
        {
            // Image paths in the GLTF are relative to the file.
            fs::path imagePath = PathFromTexID(mapModel.meshMaterial[m]);
            fs::path imagePathFromGLTF = fs::relative(
                fs::current_path() / imagePath, 
                fs::current_path() / filePath.parent_path()); 
            
            // Push material
            materials.push_back({
                {"name", imagePathFromGLTF.generic_string()},
                {"pbrMetallicRoughness", {
                    {"baseColorTexture", {
                        {"index", textures.size()},
                        {"texCoord", 0}
                    }},
                    {"metallicFactor", 0.0f},
                    {"roughnessFactor", 1.0f},
                }}
            });

            // Push texture
            textures.push_back({
                {"source", textures.size()},
                {"sampler", 0}
            });

            // Push image
            images.push_back({
                {"uri", imagePathFromGLTF.generic_string()}
            });

            if (separateGeometry)
            {
                // When separate geometry is enabled, each material gets its own node containing its portion of the map geometry
                std::string nodeName = fs::relative(fs::current_path() / imagePath, fs::current_path() / App::Get()->GetTexturesDir()).generic_string();
                
                // The compiler thinks this is necessary, apparently... 9_9
                char nodeNameBuffer[nodeName.length() + 1];
                memcpy(nodeNameBuffer, nodeName.data(), nodeName.length() + 1);
                // Replace slashes from the path with underscores. This makes sure the names are imported into Godot without modification.
                char* newNodeName = TextReplace(nodeNameBuffer, "/", "_");
                char* finalNodeName = TextReplace(newNodeName, ".", "_");

                json materialNode = {{"name", std::string(finalNodeName)}};

                free(newNodeName);
                free(finalNodeName);

                json mesh = {
                    {"primitives", {mapPrims[m]}}
                };

                materialNode["mesh"] = meshes.size();
                meshes.push_back(mesh);

                mapNodeChildren.push_back(nodes.size());
                nodes.push_back(materialNode);
            }
        }

        samplers.push_back({
            {"magFilter", FILTER_NEAREST},
            {"minFilter", FILTER_NEAREST_MIP_NEAREST},
            {"wrapS", WRAP_REPEAT},
            {"wrapT", WRAP_REPEAT},
        });

        // Everything is stored in one buffer, to make it compatible with .GLB
        size_t bufferSize = bufferOffset;
        json buffer = {{"byteLength", bufferSize}};

        bufferData = SAFE_MALLOC(uint8_t, bufferSize);

        // Fill the buffer with the mesh data, now that we know all of the counts and offsets.
        for (int i = 0; i < mapModel.meshCount; ++i)
        {
            // Position
            {
                const json& bufferView = bufferViews[i * 4 + 0];
                size_t byteOffset = (size_t) bufferView["byteOffset"];
                size_t byteLength = (size_t) bufferView["byteLength"];
                memcpy(&bufferData[byteOffset], mapModel.meshes[i].vertices, byteLength);
            }
            // Tex coord
            {
                const json& bufferView = bufferViews[i * 4 + 1];
                size_t byteOffset = (size_t) bufferView["byteOffset"];
                size_t byteLength = (size_t) bufferView["byteLength"];
                memcpy(&bufferData[byteOffset], mapModel.meshes[i].texcoords, byteLength);
            }
            // Normals
            {
                const json& bufferView = bufferViews[i * 4 + 2];
                size_t byteOffset = (size_t) bufferView["byteOffset"];
                size_t byteLength = (size_t) bufferView["byteLength"];
                memcpy(&bufferData[byteOffset], mapModel.meshes[i].normals, byteLength);
            }
            // Indices
            {
                const json& bufferView = bufferViews[i * 4 + 3];
                size_t byteOffset = (size_t) bufferView["byteOffset"];
                size_t byteLength = (size_t) bufferView["byteLength"];
                memcpy(&bufferData[byteOffset], mapModel.meshes[i].indices, byteLength);
            }
        }

        if (!isGLB)
        {
            // For plain .gltf files, simply encode the buffer into a base64 data string.
            // For .glb, the buffer will be written to the end of the binary file later.
            buffer["uri"] = std::string("data:application/octet-stream;base64,") + base64::encode(bufferData, bufferSize);
        }

        buffers.push_back(buffer);

        json mapNode = {{"name", "map"}};

        // Indices for each root node, because the scene object requires a list of them.
        std::vector<int> rootNodes;

        if (!separateGeometry)
        {
            // Assign all whole map geometry to the map node
            mapNode["mesh"] = meshes.size();
            meshes.push_back({
                {"primitives", mapPrims}
            });
        }
        else
        {
            mapNode["children"] = mapNodeChildren;
        }
        
        rootNodes.push_back(nodes.size());
        nodes.push_back(mapNode);

        // Add entities as nodes
        for (const Ent &ent : _entGrid.GetEntList())
        {
            Quaternion rot = QuaternionFromEuler(ToRadians((float)ent.pitch), ToRadians((float)ent.yaw), 0.0f);
            
            json entNode = {
                {"translation", { ent.position.x, ent.position.y, ent.position.z }},
                {"scale", { ent.radius, ent.radius, ent.radius }},
                {"rotation", { rot.x, rot.y, rot.z, rot.w }},
                {"extras", ent.properties}
            };

            if (ent.properties.find("name") != ent.properties.end())
                entNode["name"] = ent.properties.at("name");

            if (ent.model != nullptr)
                entNode["extras"]["modelPath"] = ent.model->GetPath().generic_string();
            
            if (ent.texture != nullptr)
                entNode["extras"]["texturePath"] = ent.texture->GetPath().generic_string();

            entNode["extras"]["color"] = json::array({ ent.color.r, ent.color.g, ent.color.b });
            
            rootNodes.push_back(nodes.size());
            nodes.push_back(entNode);
        }

        // Set up scene. There is only one scene and it has all of the nodes.
        json scene;
        scene["nodes"] = rootNodes;
        scenes.push_back(scene);

        // Marshall all of the data into the main JSON object.
        json jsonData = {
            {"asset", {
                {"version", "2.0"},
                {"generator", "Total Editor 3"}
            }},
            {"nodes", nodes},
            {"meshes", meshes},
            {"buffers", buffers},
            {"bufferViews", bufferViews},
            {"accessors", accessors},
            {"scenes", scenes},
            {"scene", 0},
            {"materials", materials},
            {"textures", textures},
            {"images", images},
            {"samplers", samplers}
        };

        // Write JSON to file
        std::ofstream file(filePath, isGLB ? std::ios::binary : std::ios::out);
        std::string jsonString = to_string(jsonData);
        uint32_t jsonLength = jsonString.size();

        // Number of bytes of padding needed for the JSON chunk
        int jsonPadding = (int)(ceilf((float)(jsonLength) / 4.0f) * 4.0f) - (int)jsonLength;
        uint32_t jsonChunkSize = jsonLength + jsonPadding + 8;

        // Number of bytes of padding needed for the BIN chunk
        int binPadding = (int)(ceilf((bufferSize) / 4.0f) * 4.0f) - (int)bufferSize;
        uint32_t binChunkSize = (uint32_t) (bufferSize + binPadding + 8);

        #define WRITE_BIN(data) file.write(reinterpret_cast<const char*>(&data), sizeof(data))

        // These constants need to be addressable in order for reinterpret_cast to work on them
        static const uint32_t GLB_VERSION = 0x02U;
        static const uint32_t GLB_MAGIC = 0x46546C67U;
        static const uint32_t GLB_JSON = 0x4E4F534AU;
        static const uint32_t GLB_BIN = 0x004E4942U;
        static const uint8_t SPACE = 0x20U;
        static const uint8_t ZERO = 0x00U;

        if (isGLB)
        {
            // Write preamble
            WRITE_BIN(GLB_MAGIC); // Magic number
            WRITE_BIN(GLB_VERSION); // Version
            uint32_t fileSize = 12U + jsonChunkSize + binChunkSize;
            WRITE_BIN(fileSize);

            // Write header for JSON chunk
            uint32_t jsonDataLength = jsonLength + (uint32_t)jsonPadding;
            WRITE_BIN(jsonDataLength);
            WRITE_BIN(GLB_JSON);
        }

        file.write(jsonString.c_str(), jsonLength);
        
        if (isGLB)
        {
            // Pad with spaces to align chunk with 4 byte boundary
            for (int p = 0; p < jsonPadding; ++p) 
                WRITE_BIN(SPACE);
            
            // Write BIN chunk header
            uint32_t binDataLength = bufferSize + binPadding;
            WRITE_BIN(binDataLength);
            WRITE_BIN(GLB_BIN);

            // Write data
            file.write(reinterpret_cast<char*>(bufferData), bufferSize);
            
            // Pad with zeroes to align with 4 byte boundary
            for (int p = 0; p < binPadding; ++p)
                WRITE_BIN(ZERO);
        }
        
        if (file.fail()) error = true;
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        error = true;
    }
    catch (...)
    {
        error = true;
    }

    if (bufferData != nullptr) free(bufferData);
    return !error;
}