#include "texture.h"

#include <iostream>

#include <SDL_opengl.h>
#include <SDL_image.h>

Texture::Texture(const std::string& filePath)
{
    bool error = false;

    SDL_Surface* rawSurface = IMG_Load(filePath.c_str());
    if (!rawSurface)
    {
        std::cout << "Failed to load texture " << filePath << ": " << IMG_GetError() << std::endl;
        const int IMG_SIZE = 64;
        surface = SDL_CreateRGBSurfaceWithFormat(0, IMG_SIZE, IMG_SIZE, 1, SDL_PIXELFORMAT_RGBA8888);
        const int BLOCK_SIZE = IMG_SIZE / 2;
        const int BLOCK_COLS = IMG_SIZE / BLOCK_SIZE;
        const int BLOCK_ROWS = BLOCK_COLS;
        Uint32* pixels = (Uint32*) surface->pixels;
		for (int k = 0; k < 64 * 64 / BLOCK_SIZE / BLOCK_SIZE; ++k)
        {
            for (int i = 0; i < BLOCK_SIZE; ++i)
            {
                for (int j = 0; j < BLOCK_SIZE; ++j)
                {
                    const int x = (k % BLOCK_COLS) * BLOCK_SIZE + i;
                    const int y = (k / BLOCK_COLS) * BLOCK_SIZE + j;
                    const int ofs = y * IMG_SIZE + x;
                    if ((k / BLOCK_COLS) % 2 == 0)
                    {
                        pixels[ofs] = (k % 2) == 0 ? 0xFF00FFFF : 0x000000FF;
                    }
                    else
                    {
                        pixels[ofs] = (k % 2) == 0 ? 0x000000FF : 0xFF00FFFF;
                    }
                }
            }
        }
        error = true;
    }
    else
    {
        surface = SDL_ConvertSurfaceFormat(rawSurface, SDL_PIXELFORMAT_RGBA8888, 0);
    }
    
    // constexpr const int garbageSize = 64 * 64 * 4;
    // unsigned char garbage [garbageSize];
    // for (int i = 0; i < garbageSize; ++i)
    // {
    //     garbage[i] = rand() % 255;
    // }

    glGenTextures(1, &glTexId);
    glBindTexture(GL_TEXTURE_2D, glTexId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (GLenum ass = glGetError(); ass != GL_NO_ERROR)
    {
        std::cout << "OpenGL Error in Texture Loading: " << ass << std::endl;
        error = true;
    }
    
    if (!error)
    {
        std::cout << "New texture loaded: " << filePath << std::endl;
    }

    SDL_FreeSurface(rawSurface);
}

Texture::~Texture()
{
    SDL_FreeSurface(surface);
    glDeleteTextures(1, &glTexId);
}