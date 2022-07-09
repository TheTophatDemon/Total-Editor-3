/*
Copyright (C) 2022 Alexander Lunsford
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

MenuBar::MenuBar(App::Settings &settings)
    : _settings(settings),
      _focused(false),
      _activeMenu(nullptr),
      _activeDialog(nullptr),
      _messageTimer(0.0f),
      _messagePriority(0)
{
    _menus = {
        (Menu) {
            .name = "MAP",
            .items = {
                (Item) { "NEW",          [&](){ _activeDialog.reset(new NewMapDialog()); } },
                (Item) { "OPEN",         [&]()
                    { 
                        auto callback = [](fs::path path){ App::Get()->TryOpenMap(path); };
                        _activeDialog.reset(new FileDialog("Open Map (*.te3)", { ".te3" }, callback)); 
                    } 
                },
                (Item) { "SAVE",         [&]()
                    { 
                        if (!App::Get()->GetLastSavedPath().empty())
                        {
                            App::Get()->TrySaveMap(App::Get()->GetLastSavedPath());
                        }
                        else
                        {
                            OpenSaveMapDialog();
                        }
                    } 
                },
                (Item) { "SAVE AS",      [&](){ OpenSaveMapDialog(); } },
                (Item) { "EXPORT",       [&](){ App::Get()->TryExportMap("test.gltf", true); }},
                (Item) { "EXPAND GRID",  [&](){ _activeDialog.reset(new ExpandMapDialog()); } },
                (Item) { "SHRINK GRID",  [&](){ _activeDialog.reset(new ShrinkMapDialog()); } },
            },
        },
        (Menu) {
            .name = "VIEW",
            .items = {
                (Item) { "MAP EDITOR",     [](){ App::Get()->ChangeEditorMode(App::Mode::PLACE_TILE); } },
                (Item) { "TEXTURE PICKER", [](){ App::Get()->ChangeEditorMode(App::Mode::PICK_TEXTURE); } },
                (Item) { "SHAPE PICKER",   [](){ App::Get()->ChangeEditorMode(App::Mode::PICK_SHAPE); } },
                (Item) { "ENTITY EDITOR",  [](){ App::Get()->ChangeEditorMode(App::Mode::EDIT_ENT); } },
                (Item) { "RESET CAMERA",   [](){ App::Get()->ResetEditorCamera(); } },
                (Item) { "TOGGLE PREVIEW", [](){ App::Get()->TogglePreviewing(); } },
            },
        },
        (Menu) {
            .name = "CONFIG",
            .items = {
                (Item) { "ASSET PATHS", [&](){ _activeDialog.reset(new AssetPathDialog(_settings)); } },
                (Item) { "SETTINGS",    [&](){ _activeDialog.reset(new SettingsDialog(_settings)); } },
            },
        },
        (Menu) {
            .name = "INFO",
            .items = {
                (Item) { "ABOUT",          [&](){ _activeDialog.reset(new AboutDialog()); } },
                (Item) { "KEYS/SHORTCUTS", [&](){ _activeDialog.reset(new ShortcutsDialog()); } },
                (Item) { "INSTRUCTIONS",   [&](){ _activeDialog.reset(new InstructionsDialog()); } },
            },
        }
    };

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
    _topBar = (Rectangle) { 0, 0, (float)GetScreenWidth(), 32 };

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
    const Rectangle MENU_BOUNDS = (Rectangle) { _topBar.x, _topBar.y, _topBar.width / 2.0f, _topBar.height };
    
    if (_messageTimer > 0.0f)
    {
        DrawTextEx(Assets::GetFont(), _statusMessage.c_str(), (Vector2) { MENU_BOUNDS.x + MENU_BOUNDS.width + 4, 2 }, MENUBAR_FONT_SIZE, 0.0f, WHITE);
    }

    const float BUTTON_WIDTH = (MENU_BOUNDS.width / _menus.size()) - (BUTTON_MARGIN * 2.0f);

    float x = BUTTON_MARGIN;
    for (auto &menu : _menus)
    {
        const Rectangle BUTT_RECT = (Rectangle) { MENU_BOUNDS.x + x, MENU_BOUNDS.y, BUTTON_WIDTH, MENU_BOUNDS.height }; //Heehee! Butt!

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
            Rectangle listBounds = (Rectangle) { 
                .x = BUTT_RECT.x, 
                .y = BUTT_RECT.y + BUTT_RECT.height, 
                .width = Max(BUTT_RECT.width, menu.width + BUTTON_MARGIN), 
                .height = 28.0f * menu.items.size() };
                
            _activeMenuBounds = (Rectangle) {
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