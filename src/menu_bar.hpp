/**
 * Copyright (c) 2022-present Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#ifndef MENU_BAR_H
#define MENU_BAR_H

#include <string>
#include <memory>

#include "app.hpp"
#include "dialogs.hpp"

class MenuBar 
{
public:
    MenuBar(App::Settings &settings);
    void Update();
    void Draw();
    void OpenSaveMapDialog();
    void OpenOpenMapDialog();
    void SaveMap();

    void DisplayStatusMessage(std::string message, float durationSeconds, int priority);
protected:
    App::Settings &_settings;

    std::unique_ptr<Dialog> _activeDialog;

    std::string _statusMessage;
    float _messageTimer;
    int _messagePriority;
};

#endif
