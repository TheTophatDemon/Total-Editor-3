/**
 * Copyright (c) 2022 Alexander Lunsford
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

#include "ent_mode.hpp"

#include "raylib.h"
#include "extras/raygui.h"

#include <cstring>

EntMode::EntMode()
    : _propsScroll(Vector2Zero()),
      _ent{ .color = WHITE, .radius = 1.0f, .position = Vector3Zero(), .yaw = 0, .pitch = 0 },
      _keyNameEdit(false),
      _changeConfirmed(false)
{
    memset(&_keyName, 0, TEXT_FIELD_MAX * sizeof(char));
}

EntMode::~EntMode()
{
    for (auto [k, v] : _propBuffers)
    {
        free(v);
    }
}

void EntMode::OnEnter() 
{
    _changeConfirmed = false;

    memset(&_keyName, 0, TEXT_FIELD_MAX * sizeof(char));

    for (auto [key, value] : _ent.properties)
    {
        _propEditing[key] = false;
        _propBuffers[key] = (char *)realloc(_propBuffers[key], sizeof(char) * TEXT_FIELD_MAX);
        strcpy(_propBuffers[key], _ent.properties[key].c_str());
    }
}

void EntMode::OnExit()
{
}

void EntMode::Update()
{

}

void EntMode::Draw()
{
    const Rectangle DRECT = Rectangle{ 0.0f, 32.0f, (float)GetScreenWidth(), (float)GetScreenHeight() - 32.0f };
    
    const Rectangle APPEAR_RECT = Rectangle {
        DRECT.x + 8.0f, DRECT.y + 8.0f, DRECT.width - 16.0f, 128.0f
    };

    //Appearance configuration

    //Color picker for empty entities.
    const Rectangle COLOR_RECT = Rectangle { APPEAR_RECT.x, APPEAR_RECT.y, 128.0f, APPEAR_RECT.height };
    Color newColor = GuiColorPicker(COLOR_RECT, "Color", _ent.color);
    _ent.color = newColor;

    //Radius slider
    const Rectangle RADIUS_RECT = Rectangle { 
        .x = COLOR_RECT.x + COLOR_RECT.width + 64.0f + 4.0f, 
        .y = COLOR_RECT.y + COLOR_RECT.height / 2.0f - 16.0f, 
        .width = APPEAR_RECT.width - (RADIUS_RECT.x - DRECT.x) - 32.0f, 
        .height = 32.0f 
    };
    _ent.radius = GuiSlider(RADIUS_RECT, "0.1", "10.0", _ent.radius, 0.1f, 10.0f);
    _ent.radius = floorf(_ent.radius / 0.05f) * 0.05f;
    char radiusLabel[256];
    sprintf(radiusLabel, "Radius: %.2f", _ent.radius);
    GuiLabel(Rectangle { RADIUS_RECT.x, RADIUS_RECT.y - 12 }, radiusLabel);

    //Property list
    Rectangle SCROLL_RECT = Rectangle {
        DRECT.x + 8.0f, APPEAR_RECT.y + APPEAR_RECT.height + 8.0f, DRECT.width - 16.0f, DRECT.height - 8.0f - (SCROLL_RECT.y - DRECT.y) - 64.0f - 4.0f
    };
    const float PROP_HEIGHT = 24.0f;

    //The contents of the list get expanded horizontally according to the longest value string
    int longestStringLen = 0;
    for (const auto &[k, v] : _ent.properties)
    {
        longestStringLen = Max(longestStringLen, v.length() + 1);
    }

    Rectangle scissor = GuiScrollPanel(SCROLL_RECT, "Properties", 
        Rectangle { .width = float(Max(int(SCROLL_RECT.width - 16), int(DRECT.width / 2) + longestStringLen * 12)), .height = (PROP_HEIGHT * _ent.properties.size()) + 8.0f }, 
        &_propsScroll);
    
    BeginScissorMode(scissor.x, scissor.y, scissor.width, scissor.height);

    float y = 4.0f;
    for (auto [key, val] : _ent.properties)
    {
        const Rectangle KEY_RECT = Rectangle { scissor.x + 4.0f + _propsScroll.x, scissor.y + y + _propsScroll.y, (scissor.width - 16.0f) / 2.0f, PROP_HEIGHT };
        if (GuiLabelButton(KEY_RECT, key.c_str()))
        {
            //Copy key name field when a key is clicked in the list.
            strcpy(_keyName, key.c_str());
        }
        const Rectangle VAL_RECT = Rectangle { KEY_RECT.x + KEY_RECT.width + 4.0f, KEY_RECT.y, float(Max(KEY_RECT.width, longestStringLen * 12)), PROP_HEIGHT };
        if (GuiTextBox(VAL_RECT, _propBuffers[key], TEXT_FIELD_MAX, _propEditing[key]))
        {
            _propEditing[key] = !_propEditing[key];
        }
        _ent.properties[key] = std::string(_propBuffers[key]);
        y += PROP_HEIGHT;
    }

    EndScissorMode();

    //Key add/remove widgets
    const Rectangle ADD_KEY_RECT = Rectangle { SCROLL_RECT.x, SCROLL_RECT.y + SCROLL_RECT.height, 32, 32 };
    const Rectangle REM_KEY_RECT = Rectangle { ADD_KEY_RECT.x + ADD_KEY_RECT.width + 4.0f, ADD_KEY_RECT.y, 32, 32 };
    if (GuiButton(ADD_KEY_RECT, "+"))
    {
        //Add key
        if (strlen(_keyName) > 0 && _ent.properties.find(_keyName) == _ent.properties.end())
        {
            _ent.properties[_keyName] = "";
            _propBuffers[_keyName] = (char *)malloc(sizeof(char) * TEXT_FIELD_MAX);
            memset(_propBuffers[_keyName], 0, sizeof(char) * TEXT_FIELD_MAX);
            _propEditing[_keyName] = false;
        }
    }
    else if (GuiButton(REM_KEY_RECT, "-"))
    {
        if (strlen(_keyName) > 0 && _ent.properties.find(_keyName) != _ent.properties.end())
        {
            _ent.properties.erase(_keyName);
            free(_propBuffers[_keyName]);
            _propBuffers.erase(_keyName);
            _propEditing.erase(_keyName);
        }
    }
    //Key name input field
    const Rectangle KEY_NAME_RECT = Rectangle { REM_KEY_RECT.x + REM_KEY_RECT.width + 4.0f, REM_KEY_RECT.y, DRECT.width - (KEY_NAME_RECT.x - DRECT.x) - 8.0f, 32.0f };
    if (GuiTextBox(KEY_NAME_RECT, _keyName, TEXT_FIELD_MAX, _keyNameEdit))
    {
        _keyNameEdit = !_keyNameEdit;
    }

    //Confirm button
    if (GuiButton(Rectangle { .x = DRECT.x + (DRECT.width / 2.0f) - 64.0f, .y = DRECT.y + DRECT.height - 32.0f - 8.0f, .width = 128.0f, .height = 32.0f }, "Place"))
    {
        _changeConfirmed = true;
        App::Get()->ChangeEditorMode(App::Mode::PLACE_TILE);
    }
}
