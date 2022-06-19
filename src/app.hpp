#ifndef APP_HPP
#define APP_HPP

#include "raylib.h"

#include <string>
#include <memory>

class PlaceMode;
class PickMode;
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

    //Mode implementation
    class ModeImpl 
    {
    public:
        virtual void Update() = 0;
        virtual void Draw() = 0;
        virtual void OnEnter() = 0;
        virtual void OnExit() = 0;
    };

    enum class Mode { PLACE_TILE, PLACE_ENT, PICK_TEXTURE, PICK_SHAPE };

    static App *Get();

    //Handles transition and data flow from one editor state to the next.
    void ChangeEditorMode(const Mode newMode);

    inline float       GetMouseSensitivity() { return _settings.mouseSensitivity; }
    inline size_t      GetUndoMax() { return _settings.undoMax; }
    inline std::string GetTexturesDir() { return _settings.texturesDir; };
    inline std::string GetShapesDir() { return _settings.shapesDir; } 

    Rectangle GetMenuBarRect();

    void Update();
    
    //General map file operations
    inline const MapMan &GetMapMan() const { return *_mapMan.get(); }
    void ResetEditorCamera();
    void NewMap(int width, int height, int length);
    void ResizeMap(int width, int height, int length, int ofsx, int ofsy, int ofsz);
private:
    App();

    Settings _settings;
    
    std::unique_ptr<MenuBar> _menuBar;
    std::unique_ptr<MapMan> _mapMan;

    std::unique_ptr<PlaceMode> _tilePlaceMode;
    //std::unique_ptr<PlaceMode> _entPlaceMode;
    std::unique_ptr<PickMode> _texPickMode;
    std::unique_ptr<PickMode> _shapePickMode;

    ModeImpl *_editorMode;
};

#endif