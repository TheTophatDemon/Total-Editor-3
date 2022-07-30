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

#include "menu_bar.hpp"

#include "extras/raygui.h"

#include <filesystem>
namespace fs = std::filesystem;

#include "math_stuff.hpp"
#include "text_util.hpp"
#include "assets.hpp"
#include "map_man.hpp"

#define BUTTON_MARGIN 4.0f
#define MENUBAR_FONT_SIZE 24.0f

MenuBar::MenuBar(App::Settings& settings)
    : _settings(settings),
    _focused(false),
    _activeMenu(nullptr),
    _activeDialog(nullptr),
    _messageTimer(0.0f),
    _messagePriority(0)
{
    //map menu
    Menu mapMenu = { "MAP" };
    mapMenu.items.push_back(Item{ "NEW",          [&]() { _activeDialog.reset(new NewMapDialog()); } });
    mapMenu.items.push_back(Item{ "OPEN",         [&](){
                                                    auto callback = [](fs::path path) { App::Get()->TryOpenMap(path); };
                                                    _activeDialog.reset(new FileDialog("Open Map (*.te3)", { ".te3" }, callback));
                                                }
                                            });
    mapMenu.items.push_back(Item { "SAVE",        [&]() {
                                                        if (!App::Get()->GetLastSavedPath().empty())
                                                        {
                                                            App::Get()->TrySaveMap(App::Get()->GetLastSavedPath());
                                                        }
                                                        else
                                                        {
                                                            OpenSaveMapDialog();
                                                        }
                                                    }
                                                });
    mapMenu.items.push_back(Item{ "SAVE AS",      [&]() { OpenSaveMapDialog(); } });
    mapMenu.items.push_back(Item{ "EXPORT",       [&]() { _activeDialog.reset(new ExportDialog(_settings)); } });
    mapMenu.items.push_back(Item{ "EXPAND GRID",  [&]() { _activeDialog.reset(new ExpandMapDialog()); } });
    mapMenu.items.push_back(Item{ "SHRINK GRID",  [&]() { _activeDialog.reset(new ShrinkMapDialog()); } });
    _menus.push_back(mapMenu);

    //view menu
    Menu view = { "VIEW" };
    view.items.push_back(Item{ "MAP EDITOR",     []() { App::Get()->ChangeEditorMode(App::Mode::PLACE_TILE); } });
    view.items.push_back(Item{ "TEXTURE PICKER", []() { App::Get()->ChangeEditorMode(App::Mode::PICK_TEXTURE); } });
    view.items.push_back(Item{ "SHAPE PICKER",   []() { App::Get()->ChangeEditorMode(App::Mode::PICK_SHAPE); } });
    view.items.push_back(Item{ "ENTITY EDITOR",  []() { App::Get()->ChangeEditorMode(App::Mode::EDIT_ENT); } });
    view.items.push_back(Item{ "RESET CAMERA",   []() { App::Get()->ResetEditorCamera(); } });
    view.items.push_back(Item{ "TOGGLE PREVIEW", []() { App::Get()->TogglePreviewing(); } });
    _menus.push_back(view);

    //config menu
    Menu config = { "CONFIG" };
    config.items.push_back(Item{ "ASSET PATHS", [&]() { _activeDialog.reset(new AssetPathDialog(_settings)); } });
    config.items.push_back(Item{ "SETTINGS",    [&]() { _activeDialog.reset(new SettingsDialog(_settings)); } });
    _menus.push_back(config);

    // info menu
    Menu info = { "INFO" };
    info.items.push_back(Item{ "ABOUT", [&]() { _activeDialog.reset(new AboutDialog()); } });
    info.items.push_back(Item{ "KEYS/SHORTCUTS", [&]() { _activeDialog.reset(new ShortcutsDialog()); } });
    info.items.push_back(Item{ "INSTRUCTIONS", [&]() { _activeDialog.reset(new InstructionsDialog()); } });

    _menus.push_back(info);

    for (Menu &menu : _menus)
    {
        std::string longestString;
        for (const Item &item : menu.items)
        {
            if (item.name.size() > longestString.size()) longestString = item.name;
        }
        menu.width = GetStringWidth(Assets::GetFont(), MENUBAR_FONT_SIZE, longestString);
    }
}

void MenuBar::OpenSaveMapDialog()
{
    auto callback = [](fs::path path){ App::Get()->TrySaveMap(path); };
    _activeDialog.reset(new FileDialog("Save Map (*.te3)", { ".te3" }, callback)); 
}

void MenuBar::Update()
{
    _topBar = Rectangle { 0, 0, (float)GetScreenWidth(), 32 };

    if (_messageTimer > 0.0f)
    {
        _messageTimer -= GetFrameTime();
        if (_messageTimer <= 0.0f)
        {
            _messageTimer = 0.0f;
            _messagePriority = 0;
        }
    }

    //Show exit confirmation dialog
    if (WindowShouldClose()) 
    {
        _activeDialog.reset(new CloseDialog());
    }

    if (_activeMenu)
    {
        _focused = true;
        //Clicking outside of the menu returns focus to the editor.
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), _activeMenuBounds))
        {
            _focused = false;
            _activeMenu = nullptr;
        }
    }
    else if (_activeDialog.get())
    {
        _focused = true;
    }
    else if (!IsMouseButtonDown(MOUSE_LEFT_BUTTON))
    {
        _focused = false;
    }
}

std::string MenuBar::_GetMenuString(const MenuBar::Menu &menu) const
{
    std::string output;

    for (const MenuBar::Item &item : menu.items)
    {
        output += item.name;
        if (item.name != (menu.items.end() - 1)->name) output.append(";");
    }

    return output;
}

void MenuBar::Draw()
{
    DrawRectangleGradientV(_topBar.x, _topBar.y, _topBar.width, _topBar.height, GRAY, DARKGRAY);
    const Rectangle MENU_BOUNDS = Rectangle { _topBar.x, _topBar.y, _topBar.width / 2.0f, _topBar.height };
    
    if (_messageTimer > 0.0f)
    {
        DrawTextEx(Assets::GetFont(), _statusMessage.c_str(), Vector2{ MENU_BOUNDS.x + MENU_BOUNDS.width + 4, 2 }, MENUBAR_FONT_SIZE, 0.0f, WHITE);
    }

    const float BUTTON_WIDTH = (MENU_BOUNDS.width / _menus.size()) - (BUTTON_MARGIN * 2.0f);

    float x = BUTTON_MARGIN;
    for (auto &menu : _menus)
    {
        const Rectangle BUTT_RECT = Rectangle { MENU_BOUNDS.x + x, MENU_BOUNDS.y, BUTTON_WIDTH, MENU_BOUNDS.height }; //Heehee! Butt!

        std::string name = menu.name;
        //Abbreviate tab names when window is too small.
        if (GetScreenWidth() < 928) name = name.substr(0, 4);
        GuiButton(BUTT_RECT, name.c_str());
        if (!_activeDialog.get() && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), BUTT_RECT))
        {
            if (_activeMenu == &menu) 
            {
                _activeMenu = nullptr;
            }
            else
            {
                _activeMenu = &menu;
            } 
        }

        if (_activeMenu == &menu)
        {
            std::string list = _GetMenuString(menu);
            Rectangle listBounds = Rectangle { 
                .x = BUTT_RECT.x, 
                .y = BUTT_RECT.y + BUTT_RECT.height, 
                .width = float(Max(BUTT_RECT.width, menu.width + BUTTON_MARGIN)), 
                .height = 28.0f * menu.items.size() };
                
            _activeMenuBounds = Rectangle {
                .x = listBounds.x, 
                .y = BUTT_RECT.y,
                .width = listBounds.width,
                .height = BUTT_RECT.height + listBounds.height,
            };
            
            int selectedItemIdx = GuiListView(listBounds, list.c_str(), nullptr, -1);
            if (selectedItemIdx >= 0)
            {
                menu.items[selectedItemIdx].action();
                _activeMenu = nullptr;
            }
        }

        x += BUTTON_WIDTH + BUTTON_MARGIN;
    }

    //Draw modal dialogs
    if (_activeDialog.get())
    {
        _activeMenu = nullptr;
        if (!_activeDialog->Draw()) _activeDialog.reset(nullptr);
    }
}
