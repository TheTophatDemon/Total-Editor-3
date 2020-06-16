#pragma once

#ifndef TEXTURE_H
#define TEXTURE_H

#include <glew.h>
#include <GL/gl.h>
#include <memory>
#include <SDL.h>

class Texture
{
public:
    //Constructor loads the texture from a file
    Texture(const std::string& filePath);
    ~Texture();
    inline const GLuint getID() const { return glTexId; }
    inline const int getWidth() const { return surface->w; }
    inline const int getHeight() const { return surface->h; }
protected:
    GLuint glTexId;
    SDL_Surface* surface;
};

#endif