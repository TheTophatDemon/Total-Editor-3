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

#include "app.hpp"

#include "raylib.h"
#include "raymath.h"

#include "imgui/rlImGui.h"
#include "imgui/imgui.h"

#include "assets/fonts/softball_gold_ttf.h"

#include <stdlib.h>
#include <vector>
#include <string>
#include <memory>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "assets.hpp"
#include "menu_bar.hpp"
#include "place_mode/place_mode.hpp"
#include "pick_mode/pick_mode.hpp"
#include "ent_mode.hpp"
#include "map_man.hpp"

#define SETTINGS_FILE_PATH "te3_settings.json"
#define BASE_WINDOW_TITLE "Total Editor 3"

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
    : _settings(),
    _mapMan        (std::make_unique<MapMan>()),
    _menuBar       (std::make_unique<MenuBar>(_settings)),
    _tilePlaceMode (std::make_unique<PlaceMode>(*_mapMan.get())),
    _texPickMode   (std::make_unique<TexturePickMode>(_settings)),
    _shapePickMode (std::make_unique<ShapePickMode>(_settings)),
    _entMode       (std::make_unique<EntMode>()),
    _editorMode    (_tilePlaceMode.get()),
    _lastSavedPath (),
    _previewDraw   (false),
    _quit          (false)
{
    std::filesystem::directory_entry entry { SETTINGS_FILE_PATH };
    if (entry.exists())
    {
        LoadSettings();
    }
    else
    {
        SaveSettings();
    }
}

void App::ChangeEditorMode(const App::Mode newMode) 
{
    _editorMode->OnExit();

    if (_editorMode == _texPickMode.get()) 
    {
        TexturePickMode::TexSelection selection = _texPickMode->GetPickedTextures();
        for (const std::shared_ptr<Assets::TexHandle>& tex : selection) 
        {
            if (tex != nullptr) 
            {
                _tilePlaceMode->SetCursorTextures(_texPickMode->GetPickedTextures());
                break;
            }
        }
    }
    else if (_editorMode == _shapePickMode.get() && _shapePickMode->GetPickedShape() != nullptr) 
    {
        _tilePlaceMode->SetCursorShape(_shapePickMode->GetPickedShape());
    }

    switch (newMode) 
    {
        case App::Mode::PICK_SHAPE: 
        {
            if (_editorMode == _tilePlaceMode.get()) 
            {
                _shapePickMode->SetPickedShape(_tilePlaceMode->GetCursorShape());
            }
            _editorMode = _shapePickMode.get(); 
        }
        break;
        case App::Mode::PICK_TEXTURE: 
        {
            if (_editorMode == _tilePlaceMode.get()) 
            {
                _texPickMode->SetPickedTextures(_tilePlaceMode->GetCursorTextures());
            }
            _editorMode = _texPickMode.get(); 
        }
        break;
        case App::Mode::PLACE_TILE: 
        {
            if (_editorMode == _entMode.get()) 
            {
                _tilePlaceMode->SetCursorEnt(_entMode->GetEnt());
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

    //Draw
    BeginDrawing();
    
    ClearBackground(GetBackgroundColor());

    rlImGuiBegin();
    _editorMode->Draw();
    _menuBar->Draw();
    rlImGuiEnd();

    if (!_previewDraw) DrawFPS(GetScreenWidth() - 24, 4);

	EndDrawing();
}

int main(int argc, char **argv)
{
    // Window stuff
	InitWindow(1280, 720, BASE_WINDOW_TITLE);
    SetWindowMinSize(640, 480);
    SetWindowState(FLAG_WINDOW_RESIZABLE);
	
    SetExitKey(KEY_NULL);

    // Set random seed based on system time;
    using std::chrono::high_resolution_clock;
    SetRandomSeed((int)high_resolution_clock::now().time_since_epoch().count());

    rlImGuiSetup(true);

    // ImGUI styling
    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;
    io.FontDefault = io.Fonts->AddFontFromMemoryTTF((void *) Softball_Gold_ttf, Softball_Gold_ttf_len, 24, &config, io.Fonts->GetGlyphRangesCyrillic());

    io.ConfigFlags = ImGuiConfigFlags_NavNoCaptureKeyboard;

    rlImGuiReloadFonts();

#ifdef DEBUG
    SetTraceLogLevel(LOG_WARNING);
#else
    SetTraceLogLevel(LOG_ERROR);
#endif

    App::Get()->ChangeEditorMode(App::Mode::PLACE_TILE);
    App::Get()->NewMap(100, 5, 100);

    // Main loop
	SetTargetFPS(60);
	while (!App::Get()->IsQuitting())
	{
        App::Get()->Update();
	}
    
    rlImGuiShutdown();

	CloseWindow();

	return 0;
}

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
    _lastSavedPath = "";
    SetWindowTitle(BASE_WINDOW_TITLE);
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
                _lastSavedPath = path;
                DisplayStatusMessage("Loaded .te3 map '" + path.filename().string() + "'.", 5.0f, 100);
            }
            else
            {
                DisplayStatusMessage("ERROR: Failed to load .te3 map. Check the console.", 5.0f, 100);
                return;
            }
            _tilePlaceMode->ResetCamera();
            // Set editor camera to saved position
            _tilePlaceMode->SetCameraOrientation(_mapMan->GetDefaultCameraPosition(), _mapMan->GetDefaultCameraAngles());
            _tilePlaceMode->ResetGrid();
        }
        else if (path.extension() == ".ti")
        {
            if (_mapMan->LoadTE2Map(path))
            {
                _lastSavedPath = "";
                DisplayStatusMessage("Loaded .ti map '" + path.filename().string() + "'.", 5.0f, 100);
            }
            else
            {
                DisplayStatusMessage("ERROR: Failed to load .ti map. Check the console.", 5.0f, 100);
                return;
            }
            _tilePlaceMode->ResetCamera();
            _tilePlaceMode->ResetGrid();
        }
        else
        {
            DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
            return;
        }
    }
    else
    {
        DisplayStatusMessage("ERROR: Invalid file path.", 5.0f, 100);
        return;
    }

    std::string newWindowTitle(BASE_WINDOW_TITLE " - Editing ");
    newWindowTitle += path.filename().string();
    SetWindowTitle(newWindowTitle.c_str());
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
            std::string msg = "Saved .te3 map '";
            msg += path.filename().string();
            msg += "'.";
            DisplayStatusMessage(msg, 5.0f, 100);
        }
        else
        {
            DisplayStatusMessage("ERROR: Map could not be saved. Check the console.", 5.0f, 100);
            return;
        }
    }
    else
    {
        DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
        return;
    }

    std::string newWindowTitle(BASE_WINDOW_TITLE " - Editing ");
    newWindowTitle += path.filename().string();
    SetWindowTitle(newWindowTitle.c_str());
}

void App::TryExportMap(fs::path path, bool separateGeometry)
{
    //Add correct extension if no extension is given.
    if (path.extension().empty())
    {
        path += ".gltf";
    }

    fs::directory_entry entry {path};

    if (path.extension() == ".gltf" || path.extension() == ".glb") 
    {
        if (_mapMan->ExportGLTFScene(path, separateGeometry))
        {
            DisplayStatusMessage(std::string("Exported map as ") + path.filename().string(), 5.0f, 100);
        }
        else
        {
            DisplayStatusMessage("ERROR: Map could not be exported. Check the console.", 5.0f, 100);
        }
    }
    else
    {
        DisplayStatusMessage("ERROR: Invalid file extension.", 5.0f, 100);
    }
}

void App::SaveSettings()
{
    try 
    {
        nlohmann::json jData;
        App::to_json(jData, _settings);
        std::ofstream file(SETTINGS_FILE_PATH);
        file << jData;
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error saving settings: " << ex.what() << std::endl;
    }
}

void App::LoadSettings()
{
    try
    {
        nlohmann::json jData;
        std::ifstream file(SETTINGS_FILE_PATH);
        file >> jData;
        App::from_json(jData, _settings);
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error loading settings: " << ex.what() << std::endl;
    }
}

// Specifies the defaults for each settings value.
App::Settings::Settings() 
{
    texturesDir = "assets/textures/tiles/";
    shapesDir = "assets/models/shapes/";
    undoMax = 30UL;
    mouseSensitivity = 0.5f;
    exportSeparateGeometry = false;
    cullFaces = true;
    defaultTexturePath = "assets/textures/tiles/brickwall.png";
    defaultShapePath = "assets/models/shapes/cube.obj";
    assetHideRegex = ".+_(atlas|hidden)\\..+";
}

void App::to_json(nlohmann::json& json, const App::Settings& settings)
{
    json["texturesDir"] = settings.texturesDir;
    json["shapesDir"] = settings.shapesDir;
    json["undoMax"] = settings.undoMax;
    json["mouseSensitivity"] = settings.mouseSensitivity;
    json["exportSeparateGeometry"] = settings.exportSeparateGeometry;
    json["cullFaces"] = settings.cullFaces;
    json["exportFilePath"] = settings.exportFilePath;
    json["defaultTexturePath"] = settings.defaultTexturePath;
    json["defaultShapePath"] = settings.defaultShapePath;
    json["backgroundColor"] = settings.backgroundColor;
    json["assetHideRegex"] = settings.assetHideRegex;
}

void App::from_json(const nlohmann::json& json, App::Settings& settings)
{
    // We have to do this manually instead of using the macro because weirdness.
    App::Settings defaultSettings = App::Settings();
    settings.texturesDir            = json.value("texturesDir", defaultSettings.texturesDir);
    settings.shapesDir              = json.value("shapesDir", defaultSettings.shapesDir);
    settings.undoMax                = json.value("undoMax", defaultSettings.undoMax);
    settings.mouseSensitivity       = json.value("mouseSensitivity", defaultSettings.mouseSensitivity);
    settings.exportSeparateGeometry = json.value("exportSeparateGeometry", defaultSettings.exportSeparateGeometry);
    settings.cullFaces              = json.value("cullFaces", defaultSettings.cullFaces);
    settings.exportFilePath         = json.value("exportFilePath", defaultSettings.exportFilePath);
    settings.defaultTexturePath     = json.value("defaultTexturePath", defaultSettings.defaultTexturePath);
    settings.defaultShapePath       = json.value("defaultShapePath", defaultSettings.defaultShapePath);
    settings.backgroundColor        = json.value("backgroundColor", defaultSettings.backgroundColor);
    settings.assetHideRegex         = json.value("assetHideRegex", defaultSettings.assetHideRegex);
}