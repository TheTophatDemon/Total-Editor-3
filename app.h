#pragma once

#include "entt/entt.hpp"

#include "assets.h"
#include "components.h"

#include <memory>

#include <SDL.h>
#include <SDL_opengl.h>

#ifndef APP_H
#define APP_H

class App
{
public:
    static const constexpr int WINDOW_WIDTH = 1280;
    static const constexpr int WINDOW_HEIGHT = 720;
    static const constexpr float WINDOW_ASPECT_RATIO = (float) WINDOW_WIDTH / WINDOW_HEIGHT;
    
    App();
    ~App();
    void beginLoop();
    inline const float getGlobalTime() { return gTimer; }
protected:
    SDL_Window* window;
    SDL_GLContext context;
    std::unique_ptr<Assets> assets;
    float gTimer;
};

#endif