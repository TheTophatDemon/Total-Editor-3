#include "input.h"

#include <imgui.h>
#include <imgui_impl_sdl.h>

#include "app.h"

bool Input::mouseButtonDown[];
bool Input::mouseButtonPress[];
glm::vec2 Input::mousePosition;
glm::vec2 Input::mouseMovement;
std::map<SDL_Scancode, bool> Input::keyDown = std::map<SDL_Scancode, bool>();
std::map<SDL_Scancode, bool> Input::keyPress = std::map<SDL_Scancode, bool>();

const glm::vec2 Input::getMousePosition()
{
    return mousePosition;
}
const glm::vec2 Input::getMouseMovement()
{
    return mouseMovement;
}

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

const bool Input::isMouseButtonDown(const MouseButton button)
{
    return mouseButtonDown[(int)button];
}

const bool Input::isMouseButtonPressed(const MouseButton button)
{
    return mouseButtonPress[(int)button];
}

const bool Input::isKeyDown(const SDL_Scancode key) {
    return keyDown[key];
}

const bool Input::isKeyPressed(const SDL_Scancode key) {
    return keyPress[key];
}

bool Input::pollEvents()
{
    for (bool& button : mouseButtonPress)
    {
        button = false;
    }
    for (auto& [key, val] : keyPress) {
        val = false;
    }

    mouseMovement.x = 0.0f;
    mouseMovement.y = 0.0f;

    SDL_Event event;
    while (SDL_PollEvent(&event) != 0)
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type)
        {
            case SDL_QUIT:
            {
                return false;
            }
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
            case SDL_MOUSEMOTION:
            {
                mousePosition.x = (float) event.motion.x / App::WINDOW_WIDTH;
                mousePosition.y = (float) event.motion.y / App::WINDOW_HEIGHT;
                mouseMovement.x = (float) event.motion.xrel / App::WINDOW_WIDTH;
                mouseMovement.y = (float) event.motion.yrel / App::WINDOW_HEIGHT;
                break;
            }
            case SDL_KEYDOWN:
            {
                keyDown[event.key.keysym.scancode] = true;
                keyPress[event.key.keysym.scancode] = true;
                break;
            }
            case SDL_KEYUP:
            {
                keyDown[event.key.keysym.scancode] = false;
                break;
            }
        }
    }

    return true;
}