#include "menu_bar.hpp"

#include "extras/raygui.h"

#include "math_stuff.hpp"
#include "text_util.hpp"
#include "assets.hpp"

#define BUTTON_MARGIN 4.0f

MenuBar::MenuBar()
    : _focused(false),
      _activeMenu(nullptr)
{
}

void MenuBar::Update()
{
    if (_activeMenu)
    {
        _focused = true;
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !CheckCollisionPointRec(GetMousePosition(), _activeMenuBounds))
        {
            _focused = false;
            _activeMenu = nullptr;
        }
    }
    else
    {
        _focused = false;
    }
}

void MenuBar::AddMenu(const std::string &menuName, std::initializer_list<std::string> items)
{
    Menu *menu = nullptr;
    for (Menu &m : _menus)
    {
        if (m.name == menuName) menu = &m;
    }
    if (!menu) _menus.push_back((Menu) { .name = menuName, .items = items });
    else menu->items = items;
}

std::string MenuBar::_GetMenuList(const MenuBar::Menu &menu) const
{
    std::string output;

    for (const std::string &item : menu.items)
    {
        output += item;
        if (item != *(menu.items.end() - 1)) output.append(";");
    }

    return output;
}

void MenuBar::Draw(Rectangle bounds)
{
    const float BUTTON_WIDTH = (bounds.width / _menus.size()) - (BUTTON_MARGIN * 2.0f);

    float x = BUTTON_MARGIN;
    for (auto &menu : _menus)
    {
        const Rectangle BUTT_RECT = (Rectangle) { bounds.x + x, bounds.y, BUTTON_WIDTH, bounds.height }; //Heehee! Butt!

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
            std::string list = _GetMenuList(menu);
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
            
            GuiListView(listBounds, list.c_str(), nullptr, -1);
        }

        x += BUTTON_WIDTH + BUTTON_MARGIN;
    }
}