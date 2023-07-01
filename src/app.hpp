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

#define TEXT_FIELD_MAX 512

class App
{
public:
    struct Settings 
    {
        std::string texturesDir;
        std::string shapesDir;
        size_t undoMax;
        float mouseSensitivity;
        bool exportSeparateGeometry; //For GLTF export
        std::string exportFilePath; //For GLTF export
        std::string defaultTexturePath;
        std::string defaultShapePath;
        size_t framesPerPage;
    };
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        Settings, 
        texturesDir, 
        shapesDir, 
        undoMax, 
        mouseSensitivity, 
        exportSeparateGeometry, 
        exportFilePath, 
        defaultTexturePath, 
        defaultShapePath, 
        framesPerPage);

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
    inline std::string GetDefaultTexturePath() { return _settings.defaultTexturePath; }
    inline std::string GetDefaultShapePath() { return _settings.defaultShapePath; }
    inline size_t      GetFramesPerPage() { return _settings.framesPerPage; }

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
