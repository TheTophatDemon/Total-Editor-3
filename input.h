#pragma once

#include <map>
#include <SDL.h>
#include <glm.hpp>
#include <glm/vec2.hpp>

#ifndef INPUT_H
#define INPUT_H

//Handles SDL events and keeps track of keyboard and mouse
class Input
{
public:
    enum class MouseButton {
        LEFT = 1, RIGHT = 3, MIDDLE = 2
    };
    //Initializes static arrays and stuff
    static void init();
    //Reads SDL Input events and updates variables. Returns false if application should close.
    static bool pollEvents();

    static const glm::vec2 getMousePosition();
    static const glm::vec2 getMouseMovement();
    static const bool isMouseButtonDown(const MouseButton button);
    static const bool isMouseButtonPressed(const MouseButton button);
    static const bool isKeyDown(const SDL_Scancode);
    static const bool isKeyPressed(const SDL_Scancode);
protected:
    static bool mouseButtonDown[256];
    static bool mouseButtonPress[256];
    static glm::vec2 mousePosition;
    static glm::vec2 mouseMovement;
    static std::map<SDL_Scancode, bool> keyDown;
    static std::map<SDL_Scancode, bool> keyPress;
};

#endif