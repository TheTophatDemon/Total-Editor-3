#include "app.hpp"

#include "raylib.h"
#include "raymath.h"

#define RAYGUI_IMPLEMENTATION
#include "extras/raygui.h"

#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>

#include "assets.hpp"
#include "menu_bar.hpp"
#include "place_mode.hpp"
#include "pick_mode.hpp"

static App *_appInstance = nullptr;

App *App::Get()
{
    if (!_appInstance)
    {
        _appInstance = new App();
    }
    return _appInstance;
}

App::App()
    : _settings { 
        .texturesDir = "assets/textures",
        .shapesDir = "assets/models/shapes/",
        .undoMax = 30UL,
        .mouseSensitivity = 0.5f
    },
    _menuBar       (std::make_unique<MenuBar>(_settings)),
    _tilePlaceMode (std::make_unique<PlaceMode>(PlaceMode::Mode::TILES)),
    _entPlaceMode  (std::make_unique<PlaceMode>(PlaceMode::Mode::ENTS)),
    _texPickMode   (std::make_unique<PickMode>(PickMode::Mode::TEXTURES)),
    _shapePickMode (std::make_unique<PickMode>(PickMode::Mode::SHAPES)),

    _editorMode(_tilePlaceMode.get())
{
}

void App::ChangeEditorMode(const App::Mode newMode) 
{
    _editorMode->OnExit();
    switch (newMode)
    {
        case App::Mode::PICK_SHAPE: 
        {
            _editorMode = _shapePickMode.get(); 
        }
        break;
        case App::Mode::PICK_TEXTURE: 
        {
            _editorMode = _texPickMode.get(); 
        }
        break;
        case App::Mode::PLACE_TILE: 
        {
            if (_editorMode == _texPickMode.get() && _texPickMode->GetPickedTexture()) 
            {
                _tilePlaceMode->SetCursorTexture(_texPickMode->GetPickedTexture());
            }
            else if (_editorMode == _shapePickMode.get() && _shapePickMode->GetPickedShape()) 
            {
                _tilePlaceMode->SetCursorShape(_shapePickMode->GetPickedShape());
            }
            _editorMode = _tilePlaceMode.get(); 
        }
        break;
        case App::Mode::PLACE_ENT: 
        {
            _editorMode = _entPlaceMode.get();   
        }
        break;
    }
    _editorMode->OnEnter();
}

Rectangle App::GetMenuBarRect()
{
    return _menuBar->GetTopBar();
}

void App::ResetEditorCamera()
{
    if (_editorMode == _tilePlaceMode.get())    _tilePlaceMode->ResetCamera();
    else if (_editorMode == _entPlaceMode.get()) _entPlaceMode->ResetCamera();
}

void App::Update()
{
    _menuBar->Update();

    if (!_menuBar->IsFocused()) 
    {
        //Mode switching hotkeys
        if (IsKeyPressed(KEY_TAB))
        {
            if (_editorMode == _tilePlaceMode.get())
            {
                if (IsKeyDown(KEY_LEFT_SHIFT)) ChangeEditorMode(Mode::PICK_SHAPE);
                else ChangeEditorMode(Mode::PICK_TEXTURE);
            }
            else if (_editorMode == _texPickMode.get())
            {
                if (IsKeyDown(KEY_LEFT_SHIFT)) ChangeEditorMode(Mode::PICK_SHAPE);
                else ChangeEditorMode(Mode::PLACE_TILE);
            }
            else if (_editorMode == _shapePickMode.get())
            {
                if (IsKeyDown(KEY_LEFT_SHIFT)) ChangeEditorMode(Mode::PICK_TEXTURE);
                else ChangeEditorMode(Mode::PLACE_TILE);
            }
        }
        
        _editorMode->Update();
    }

    //Draw
    BeginDrawing();
    
    ClearBackground(BLACK);

    _editorMode->Draw();
    _menuBar->Draw();

    DrawFPS(4, GetScreenHeight() - 24);

	EndDrawing();
}

int main(int argc, char **argv)
{
    //Window stuff
	InitWindow(1280, 720, "Total Editor 3");
    SetWindowMinSize(640, 480);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
	InitAudioDevice();
    SetExitKey(KEY_NULL);

    Assets::Initialize();

    //RayGUI Styling
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(DARKGRAY));
    GuiSetFont(*Assets::GetFont());
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(LABEL, TEXT_COLOR_FOCUSED, ColorToInt(YELLOW));
    GuiSetStyle(LABEL, TEXT_COLOR_PRESSED, ColorToInt(LIGHTGRAY));
    GuiSetStyle(SCROLLBAR, SCROLL_SPEED, 64);
    GuiSetStyle(LISTVIEW, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(VALUEBOX, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));

    //Main loop
	SetTargetFPS(300);
	while (!WindowShouldClose())
	{
        Assets::Update();
        App::Get()->Update();
	}
    
    //Deinitialization
    Assets::Unload();
	CloseWindow();

	return 0;
}
