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

#ifndef PICK_MODE_H
#define PICK_MODE_H

#include "raylib.h"
#include "imgui/imgui.h"

#include <vector>
#include <string>
#include <map>
#include <set>

#include "../app.hpp"
#include "../dialogs/dialogs.hpp"

#define SEARCH_BUFFER_SIZE 256

class PickMode : public App::ModeImpl
{
public:
    //Represents a selectable frame in the list or grid of the picker
    struct Frame 
    {
        fs::path        filePath;
        std::string     label;
        Texture         texture;

        Frame();
        Frame(const fs::path filePath, const fs::path rootDir);
    };

    PickMode(App::Settings &settings, int maxSelectionCount, std::string fileExtension);
    virtual void OnEnter() override;
    virtual void Update() override;
    virtual void Draw() override;
protected:
    virtual Texture GetFrameTexture(const fs::path& filePath) = 0;
    virtual void SelectFrame(const Frame frame) = 0;

    // Retrieves files, recursively, and generates frames for each.
    
    App::Settings &_settings;
    std::set<fs::path> _foundFiles;
    std::vector<Frame> _frames;
    fs::path _rootDir;
    std::string _fileExtension;
    int _maxSelectionCount;
    
private:
    void _GetFrames();

    std::unique_ptr<Dialog> _activeDialog;
    char _searchFilterBuffer[SEARCH_BUFFER_SIZE];
    char _searchFilterPrevious[SEARCH_BUFFER_SIZE];
};

class TexturePickMode : public PickMode
{
public:
    using TexSelection = std::array<std::shared_ptr<Assets::TexHandle>, TEXTURES_PER_TILE>;

    TexturePickMode(App::Settings &settings);
    virtual void OnEnter() override;
    virtual void OnExit() override;

    TexSelection GetPickedTextures() const;
    void SetPickedTextures(TexSelection newTextures);
protected:
    virtual Texture GetFrameTexture(const fs::path& filePath) override;
    virtual void SelectFrame(const Frame frame) override;

    std::map<fs::path, Texture2D> _loadedTextures;
    TexSelection _selectedTextures;
};

class ShapePickMode : public PickMode
{
public:
    ShapePickMode(App::Settings &settings);
    virtual void OnEnter() override;
    virtual void Update() override;
    virtual void OnExit() override;

    std::shared_ptr<Assets::ModelHandle> GetPickedShape() const;
    void SetPickedShape(std::shared_ptr<Assets::ModelHandle> newModel);
protected:
    virtual Texture GetFrameTexture(const fs::path& filePath) override;
    virtual void SelectFrame(const Frame frame) override;
    
    // Load or retrieve cached model
    Model _GetModel(const fs::path path);
    // Load or retrieve cached render texture
    RenderTexture2D _GetShapeIcon(const fs::path path);

    Camera _iconCamera; // Camera for rendering 3D shape preview icons

    std::shared_ptr<Assets::ModelHandle> _selectedShape;
    std::map<fs::path, Model> _loadedModels;
    std::map<fs::path, RenderTexture2D> _loadedIcons;
};

#endif
