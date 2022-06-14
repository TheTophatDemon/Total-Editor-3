#ifndef MENU_BAR_H
#define MENU_BAR_H

#include "raylib.h"

#include <initializer_list>
#include <string>
#include <map>
#include <vector>

class MenuBar 
{
public:
    MenuBar();
    void Update();
    void Draw(Rectangle bounds);

    void AddMenu(const std::string &menuName, std::initializer_list<std::string> items);

    inline bool IsFocused() const { return _focused; }
protected:
    struct Menu 
    {
        std::string name;
        std::vector<std::string> items;
    };

    //Turns the menu's item list into a single string of semicolon separated names for use with RayGUI
    std::string _GetMenuList(const Menu &menu) const;

    std::vector<Menu> _menus;
    Menu *_activeMenu;
    Rectangle _activeMenuBounds;
    bool _focused;
};

#endif