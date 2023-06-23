/**
 * Copyright (c) 2022 Alexander Lunsford
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

#include "app.hpp"

#define SEARCH_BUFFER_SIZE 256

class PickMode : public App::ModeImpl
{
public:
    //Represents a selectable frame in the list or grid of the picker
    struct Frame 
    {
        std::shared_ptr<Assets::TexHandle>   tex;
        std::shared_ptr<Assets::ModelHandle> shape;
        RenderTexture2D                      renderTex; //Used to render model icons
        std::string                          label;

        inline ~Frame() 
        {
            if (renderTex.texture.width != 0 && renderTex.texture.height != 0)
            {
                UnloadRenderTexture(renderTex);
            }
        }
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

    inline std::shared_ptr<Assets::TexHandle> GetPickedTexture() const
    { 
        assert(_mode == Mode::TEXTURES);
        if (!_selectedFrame) return nullptr;
        return _selectedFrame->tex;
    }

    inline std::shared_ptr<Assets::ModelHandle> GetPickedShape() const
    {
        assert(_mode == Mode::SHAPES);
        if (!_selectedFrame) return nullptr;
        return _selectedFrame->shape;
    }

protected:
    //Retrieves files, recursively, and generates frames for each.
    void _GetFrames(fs::path rootDir);

    void _DrawGridView(Rectangle framesView);
    void _DrawListView(Rectangle framesView);
    void _DrawFrame(Frame *frame, Rectangle rect);

    std::vector<Frame> _frames;
    std::vector<Frame*> _filteredFrames;
    Frame* _selectedFrame;
    size_t _longestLabelLength;
    
    char _searchFilterBuffer[SEARCH_BUFFER_SIZE];
    bool _searchFilterFocused;

    Mode _mode;
    View _view;
    Vector2 _scroll;
};

#endif
