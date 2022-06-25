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
#include "ent_mode.hpp"
#include "map_man.hpp"

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
    _mapMan        (std::make_unique<MapMan>()),
    _tilePlaceMode (std::make_unique<PlaceMode>(*_mapMan.get())),
    _texPickMode   (std::make_unique<PickMode>(PickMode::Mode::TEXTURES)),
    _shapePickMode (std::make_unique<PickMode>(PickMode::Mode::SHAPES)),
    _entMode       (std::make_unique<EntMode>()),
    _editorMode    (_tilePlaceMode.get()),
    _previewDraw   (false),
    _lastSavedPath (),
    _menuBar       (std::make_unique<MenuBar>(_settings)),
    _quit          (false)
{
}

void App::ChangeEditorMode(const App::Mode newMode) 
{
    _editorMode->OnExit();

    if (_editorMode == _texPickMode.get() && _texPickMode->GetPickedTexture() != NO_TEX) 
    {
        _tilePlaceMode->SetCursorTexture(_texPickMode->GetPickedTexture());
    }
    else if (_editorMode == _shapePickMode.get() && _shapePickMode->GetPickedShape() != NO_MODEL) 
    {
        _tilePlaceMode->SetCursorShape(_shapePickMode->GetPickedShape());
    }

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
            if (_editorMode == _entMode.get())
            {
                if (_entMode->IsChangeConfirmed()) _tilePlaceMode->SetCursorEnt(_entMode->GetEnt());
            }
            _editorMode = _tilePlaceMode.get(); 
        }
        break;
        case App::Mode::EDIT_ENT:
        {
            if (_editorMode == _tilePlaceMode.get()) _entMode->SetEnt(_tilePlaceMode->GetCursorEnt());
            _editorMode = _entMode.get();
        }
        break;
    }
    _editorMode->OnEnter();
}

void App::Update()
{
    _menuBar->Update();

    if (!_menuBar->IsFocused()) 
    {
        //Mode switching hotkeys
        if (IsKeyPressed(KEY_TAB))
        {
            if (IsKeyDown(KEY_LEFT_SHIFT))
            {
                if (_editorMode == _tilePlaceMode.get()) ChangeEditorMode(Mode::PICK_SHAPE);
                else ChangeEditorMode(Mode::PLACE_TILE);
            }
            else if (IsKeyDown(KEY_LEFT_CONTROL))
            {
                if (_editorMode == _tilePlaceMode.get()) ChangeEditorMode(Mode::EDIT_ENT);
                else if (_editorMode == _entMode.get()) ChangeEditorMode(Mode::PLACE_TILE);
            }
            else
            {
                if (_editorMode == _tilePlaceMode.get()) ChangeEditorMode(Mode::PICK_TEXTURE);
                else ChangeEditorMode(Mode::PLACE_TILE);
            }
        }

        //Save hotkey
        if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
        {
            if (!GetLastSavedPath().empty())
            {
                TrySaveMap(GetLastSavedPath());
            }
            else
            {
                _menuBar->OpenSaveMapDialog();
            }
        }
        
        _editorMode->Update();
    }

    //Draw
    Assets::RedrawIcons(); //This must be done before BeginDrawing() for some reason.

    BeginDrawing();
    
    ClearBackground(BLACK);

    _editorMode->Draw();
    _menuBar->Draw();

    if (!_previewDraw) DrawFPS(4, GetScreenHeight() - 24);

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

    //Set random seed based on system time;
    using std::chrono::high_resolution_clock;
    SetRandomSeed((int)high_resolution_clock::now().time_since_epoch().count());

    //RayGUI Styling
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, ColorToInt(DARKGRAY));
    GuiSetFont(Assets::GetFont());
    GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(LABEL, TEXT_COLOR_FOCUSED, ColorToInt(YELLOW));
    GuiSetStyle(LABEL, TEXT_COLOR_PRESSED, ColorToInt(LIGHTGRAY));
    GuiSetStyle(SCROLLBAR, SCROLL_SPEED, 64);
    GuiSetStyle(LISTVIEW, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(LISTVIEW, TEXT_ALIGNMENT, TEXT_ALIGN_LEFT);
    GuiSetStyle(VALUEBOX, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(DROPDOWNBOX, TEXT_COLOR_NORMAL, ColorToInt(BLACK));
    GuiSetStyle(TEXTBOX, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));
    GuiSetStyle(TEXTBOX, BACKGROUND_COLOR, ColorToInt(DARKGRAY));
    GuiSetStyle(SLIDER, TEXT_COLOR_NORMAL, ColorToInt(RAYWHITE));

    App::Get()->NewMap(100, 5, 100);

    //Main loop
	SetTargetFPS(300);
	while (!App::Get()->IsQuitting())
	{
        App::Get()->Update();
	}
    
	CloseWindow();

	return 0;
}

Rectangle App::GetMenuBarRect() { return _menuBar->GetTopBar(); }

void App::DisplayStatusMessage(std::string message, float durationSeconds, int priority)
{
    _menuBar->DisplayStatusMessage(message, durationSeconds, priority);
}

void App::ResetEditorCamera()
{
    if (_editorMode == _tilePlaceMode.get()) 
    {
        _tilePlaceMode->ResetCamera();
        _tilePlaceMode->ResetGrid();
    }
}

void App::NewMap(int width, int height, int length)
{
    _mapMan->NewMap(width, height, length);
    _tilePlaceMode->ResetCamera();
    _tilePlaceMode->ResetGrid();
}

void App::ExpandMap(Direction axis, int amount)
{
    _mapMan->ExpandMap(axis, amount);
    _tilePlaceMode->ResetGrid();
}

void App::ShrinkMap()
{
    _mapMan->ShrinkMap();
    _tilePlaceMode->ResetGrid();
    _tilePlaceMode->ResetCamera();
}

void App::TryOpenMap(fs::path path)
{
    fs::directory_entry entry {path};
    if (entry.exists() && entry.is_regular_file())
    {
        if (path.extension() == ".te3") 
        {
            if (_mapMan->LoadTE3Map(path))
            {
                DisplayStatusMessage("Loaded .te3 map.", 5.0f, 100);
            }
            else
            {
                DisplayStatusMessage("ERROR: Failed to load .te3 map. Check the console.", 5.0f, 100);
            }
            _tilePlaceMode->ResetCamera();
            _tilePlaceMode->ResetGrid();
        }
        else if (path.extension() == ".ti")
        {
            DisplayStatusMessage("Loaded .ti map.", 5.0f, 100);
        }
        else
        {
            DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
        }
    }
    else
    {
        DisplayStatusMessage("ERROR: Invalid file path.", 5.0f, 100);
    }
}

void App::TrySaveMap(fs::path path)
{
    //Add correct extension if no extension is given.
    if (path.extension().empty())
    {
        path += ".te3";
    }

    fs::directory_entry entry {path};

    if (path.extension() == ".te3") 
    {
        if (_mapMan->SaveTE3Map(path))
        {
            _lastSavedPath = path;
            DisplayStatusMessage("Saved .te3 map.", 5.0f, 100);
        }
        else
        {
            DisplayStatusMessage("ERROR: Map could not be saved. Check the console.", 5.0f, 100);
        }
    }
    else
    {
        DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
    }
}