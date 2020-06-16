#pragma once

#include <memory>
#include <unordered_map>

#include <SDL.h>

#include "texture.h"

class Assets
{
public:
    typedef std::unordered_map<std::string, std::shared_ptr<Texture>> TexMap;
    Assets(const char* directory);
	
	///Retrieves a texture from the cache, or loads it in if it isn't there.
    std::shared_ptr<Texture> getTexture(const std::string path);
protected:
    std::string directory;
    std::unordered_map<std::string, std::shared_ptr<Texture>> textureMap;
};