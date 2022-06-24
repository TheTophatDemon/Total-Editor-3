#ifndef MENU_BAR_H
#define MENU_BAR_H

#include "raylib.h"

#include <initializer_list>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

#include "app.hpp"
#include "dialogs.hpp"

class MenuBar 
{
public:
    struct Item
    {
        std::string name;
        std::function<void()> action;
    };

    struct Menu
    {
        std::string name;
        std::vector<Item> items;
        size_t width;
    };

    MenuBar(App::Settings &settings);
    void Update();
    void Draw();
    void OpenSaveMapDialog();

    inline bool IsMouseOn() const { return _mouseOn; }
    inline bool IsFocused() const { return _focused; }
    inline Rectangle GetTopBar() const { return _topBar; }

    inline void DisplayStatusMessage(std::string message, float durationSeconds, int priority)
    {
        if (priority >= _messagePriority)
        {
            _statusMessage = message;
            _messageTimer = durationSeconds;
        }
    }
protected:
    //Turns the menu's item list into a single string of semicolon separated names for use with RayGUI
    std::string _GetMenuString(const Menu &menu) const;

    App::Settings &_settings;
    Rectangle _topBar;

    std::vector<Menu> _menus;
    Menu *_activeMenu;
    std::unique_ptr<Dialog> _activeDialog;
    Rectangle _activeMenuBounds;

    std::string _statusMessage;
    float _messageTimer;
    int _messagePriority;

    bool _focused;
    bool _mouseOn;
};

#endif