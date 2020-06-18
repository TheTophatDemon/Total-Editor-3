#pragma once

#include <SDL.h>

#ifndef INPUT_H
#define INPUT_H

//Handles SDL events and keeps track of keyboard and mouse
class Input
{
public:
    static void init();
    //Reads SDL Input events. Returns false if application should close.
    static bool pollEvents();
protected:
    static bool mouseButtonDown[256];
    static bool mouseButtonPress[256];
};

#endif