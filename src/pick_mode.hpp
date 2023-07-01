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

#include <vector>
#include <string>
#include <assert.h>
#include <mutex>
#include <future>
#include <map>

#include "app.hpp"

#define SEARCH_BUFFER_SIZE 256

class PickMode : public App::ModeImpl
{
public:
    //Represents a selectable frame in the list or grid of the picker
    struct Frame 
    {
        fs::path        filePath;
        std::string     label;

        Frame();
        Frame(const fs::path filePath, const fs::path rootDir);
    };

    enum class Mode { TEXTURES, SHAPES };
    enum class View { GRID, LIST };

    PickMode(Mode mode);
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    
    inline Mode GetMode() const { return _mode; }
    inline View GetView() const { return _view; }

    std::shared_ptr<Assets::TexHandle> GetPickedTexture() const;
    std::shared_ptr<Assets::ModelHandle> GetPickedShape() const;

protected:
    //Retrieves files, recursively, and generates frames for each.
    void _GetFrames(fs::path rootDir);

    void _DrawGridView(Rectangle framesView);
    void _DrawListView(Rectangle framesView);
    void _DrawFrame(Frame& frame, Rectangle rect);

    Texture2D _GetTexture(const fs::path path);
    Model _GetModel(const fs::path path);
    RenderTexture2D _GetIcon(const fs::path path);

    std::map<fs::path, Texture2D> _loadedTextures;
    std::map<fs::path, Model> _loadedModels;
    std::map<fs::path, RenderTexture2D> _loadedIcons;
    std::vector<fs::path> _foundFiles;
    std::vector<Frame> _frames;
    
    Frame _selectedFrame;
    size_t _longestLabelLength;
    Camera _iconCamera; //Camera for rendering 3D shape preview icons
    
    char _searchFilterBuffer[SEARCH_BUFFER_SIZE];
    char _searchFilterPrevious[SEARCH_BUFFER_SIZE];
    bool _searchFilterFocused;

    Mode _mode;
    View _view;
    Vector2 _scroll;
};

#endif
