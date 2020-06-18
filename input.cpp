#include "input.h"

#include <glm.hpp>

#include <imgui.h>
#include <imgui_impl_sdl.h>

bool Input::mouseButtonDown[];
bool Input::mouseButtonPress[];

void Input::init()
{
    for (bool& button : mouseButtonDown)
    {
        button = false;
    }
    for (bool& button : mouseButtonPress)
    {
        button = false;
    }
}

bool Input::pollEvents()
{
    for (bool& button : mouseButtonPress)
    {
        button = false;
    }

    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
                return false;
            case SDL_MOUSEBUTTONDOWN:
            {
                const Uint8 mouseButton = glm::min<int>(event.button.button, 255);
                mouseButtonDown[mouseButton] = true;
                mouseButtonPress[mouseButton] = true;
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                const Uint8 mouseButton = glm::min<int>(event.button.button, 255);
                mouseButtonDown[mouseButton] = false;
                break;
            }
        }
    }

    return true;
}