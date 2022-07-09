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

#ifndef APP_HPP
#define APP_HPP

#include "raylib.h"
#include "json.hpp"

#include <string>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;

#include "tile.hpp"

class PlaceMode;
class PickMode;
class EntMode;
class MenuBar;
class MapMan;

class App
{
public:
    struct Settings 
    {
        std::string texturesDir;
        std::string shapesDir;
        size_t undoMax;
        float mouseSensitivity;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Settings, texturesDir, shapesDir, undoMax, mouseSensitivity);

    //Mode implementation
    class ModeImpl 
    {
    public:
        inline virtual ~ModeImpl() {}
        virtual void Update() = 0;
        virtual void Draw() = 0;
        virtual void OnEnter() = 0;
        virtual void OnExit() = 0;
    };

    enum class Mode { PLACE_TILE, PICK_TEXTURE, PICK_SHAPE, EDIT_ENT };

    static App *Get();

    //Handles transition and data flow from one editor state to the next.
    void ChangeEditorMode(const Mode newMode);

    inline float       GetMouseSensitivity() { return _settings.mouseSensitivity; }
    inline size_t      GetUndoMax() { return _settings.undoMax; }
    inline std::string GetTexturesDir() { return _settings.texturesDir; };
    inline std::string GetShapesDir() { return _settings.shapesDir; } 

    //Indicates if rendering should be done in "preview mode", i.e. without editor widgets being drawn.
    inline bool IsPreviewing() const { return _previewDraw; }
    inline void SetPreviewing(bool p) { _previewDraw = p; }
    inline void TogglePreviewing() { _previewDraw = !_previewDraw; }
    inline fs::path GetLastSavedPath() const { return _lastSavedPath; }

    inline bool IsQuitting() const { return _quit; }
    inline void Quit() { _quit = true; }

    Rectangle GetMenuBarRect();
    void DisplayStatusMessage(std::string message, float durationSeconds, int priority);

    void Update();
    
    //General map file operations
    inline const MapMan &GetMapMan() const { return *_mapMan.get(); }
    void ResetEditorCamera();
    void NewMap(int width, int height, int length);
    void ExpandMap(Direction axis, int amount);
    void ShrinkMap();
    void TryOpenMap(fs::path path);
    void TrySaveMap(fs::path path);
    void TryExportMap(fs::path path, bool separateGeometry);

    //Serializes settings into JSON file and exports.
    void SaveSettings();
    //Deserializes settings from JSON file
    void LoadSettings();
private:
    App();

    Settings _settings;
    
    std::unique_ptr<MenuBar> _menuBar;
    std::unique_ptr<MapMan> _mapMan;

    std::unique_ptr<PlaceMode> _tilePlaceMode;
    std::unique_ptr<PickMode> _texPickMode;
    std::unique_ptr<PickMode> _shapePickMode;
    std::unique_ptr<EntMode> _entMode;

    ModeImpl *_editorMode;

    fs::path _lastSavedPath;
    bool _previewDraw;

    bool _quit;
};

#endif