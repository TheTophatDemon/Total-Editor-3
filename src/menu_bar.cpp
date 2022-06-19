#include "menu_bar.hpp"

#include "extras/raygui.h"

#include "math_stuff.hpp"
#include "text_util.hpp"
#include "assets.hpp"

#define BUTTON_MARGIN 4.0f

MenuBar::MenuBar(App::Settings &settings)
    : _settings(settings),
      _focused(false),
      _activeMenu(nullptr),
      _activeDialog(nullptr)
{
    _menus = {
        (Menu) {
            .name = "MAP",
            .items = {
                (Item) { "NEW",          [&](){ _activeDialog.reset(new NewMapDialog()); } },
                (Item) { "OPEN",         [&](){ _activeDialog = nullptr; } },
                (Item) { "SAVE",         [&](){ _activeDialog = nullptr; } },
                (Item) { "SAVE AS",      [&](){ _activeDialog = nullptr; } },
                (Item) { "EXPAND GRID",  [&](){ _activeDialog.reset(new ExpandMapDialog()); } },
                (Item) { "SHRINK GRID",  [&](){ _activeDialog = nullptr; } },
            }
        },
        (Menu) {
            .name = "VIEW",
            .items = {
                (Item) { "MAP EDITOR",     [](){ App::Get()->ChangeEditorMode(App::Mode::PLACE_TILE); } },
                (Item) { "TEXTURE PICKER", [](){ App::Get()->ChangeEditorMode(App::Mode::PICK_TEXTURE); } },
                (Item) { "SHAPE PICKER",   [](){ App::Get()->ChangeEditorMode(App::Mode::PICK_SHAPE); } },
                (Item) { "THINGS",         [](){ App::Get()->ChangeEditorMode(App::Mode::PLACE_ENT); } },
                (Item) { "RESET CAMERA",   [](){ App::Get()->ResetEditorCamera(); } },
                (Item) { "TOGGLE GRID",    [](){ ; } },
            }
        },
        (Menu) {
            .name = "CONFIG",
            .items = {
                (Item) { "ASSET PATHS", [](){} },
                (Item) { "SETTINGS",    [](){} },
            }
        },
        (Menu) {
            .name = "INFO",
            .items = {
                (Item) { "ABOUT",        [](){} },
                (Item) { "SHORTCUTS",    [](){} },
                (Item) { "INSTRUCTIONS", [](){} },
            }
        }
    };
}

void MenuBar::Update()
{
    _topBar = (Rectangle) { 0, 0, (float)GetScreenWidth(), 32 };

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
    
    // if (_layerViewMin > 0 || _layerViewMax < _tileGrid.GetHeight() - 1)
    // {
    //     DrawTextEx(*Assets::GetFont(), "PRESS H TO UNHIDE LAYERS", (Vector2) { MENU_BAR_RECT.x + MENU_BAR_RECT.width + 4, 2 }, 24, 0.0f, WHITE);
    // }

    const float BUTTON_WIDTH = (MENU_BOUNDS.width / _menus.size()) - (BUTTON_MARGIN * 2.0f);

    float x = BUTTON_MARGIN;
    for (auto &menu : _menus)
    {
        const Rectangle BUTT_RECT = (Rectangle) { MENU_BOUNDS.x + x, MENU_BOUNDS.y, BUTTON_WIDTH, MENU_BOUNDS.height }; //Heehee! Butt!

        GuiButton(BUTT_RECT, menu.name.c_str());
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), BUTT_RECT))
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
                .width = Max(BUTT_RECT.width, GetStringWidth(*Assets::GetFont(), list) + BUTTON_MARGIN), 
                .height = BUTT_RECT.height * menu.items.size() };
                
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
        if (!_activeDialog->Draw()) _activeDialog.reset(nullptr);
    }
}