#pragma once

#include <SDL.h>
#include <glm.hpp>
#include <glm/vec2.hpp>

#ifndef INPUT_H
#define INPUT_H

//Handles SDL events and keeps track of keyboard and mouse
class Input
{
public:
    //Initializes static arrays and stuff
    static void init();
    //Reads SDL Input events and updates variables. Returns false if application should close.
    static bool pollEvents();

    static const glm::vec2 getMousePosition();
    static const glm::vec2 getMouseMovement();
    static const bool getMouseButtonDown(const Uint8 button);
    static const bool getMouseButtonPressed(const Uint8 button);
protected:
    static bool mouseButtonDown[256];
    static bool mouseButtonPress[256];
    static glm::vec2 mousePosition;
    static glm::vec2 mouseMovement;
};

#endif