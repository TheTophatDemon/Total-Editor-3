#include "assets.h"

#include <iostream>

Assets::Assets(const char* directory) : directory(directory),
    textureMap(std::unordered_map<std::string, std::shared_ptr<Texture>>()) 
{}

std::shared_ptr<Texture> Assets::getTexture(const std::string path)
{
    //Try to find the texture in the existing dictionary
    TexMap::const_iterator texLoc = textureMap.find(path);
    if (texLoc != textureMap.end())
    {
        return texLoc->second;
    }
    
    //Otherwise, add a new texture to the dictionary
    std::string fullPath (std::string(directory).append("\\").append(path));
    auto tex = std::make_shared<Texture>(fullPath);
    textureMap.emplace(path, tex);
    return tex;
}