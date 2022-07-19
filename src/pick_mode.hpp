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

#ifndef PICK_MODE_H
#define PICK_MODE_H

#include "raylib.h"

#include <vector>
#include <string>
#include <assert.h>

#include "app.hpp"

#define SEARCH_BUFFER_SIZE 256

class PickMode : public App::ModeImpl {
public:
    struct Frame {
        Texture2D tex;
        ModelID shape;
        std::string label;
    };

    enum class Mode { TEXTURES, SHAPES };
    enum class View { GRID, LIST };

    PickMode(Mode mode);
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    
    // inline void SetMode(Mode mode) { _mode = mode; }
    inline Mode GetMode() const { return _mode; }
    inline View GetView() const { return _view; }

    inline TexID GetPickedTexture() const
    { 
        assert(_mode == Mode::TEXTURES);
        if (!_selectedFrame) return NO_TEX;
        //The texture is only loaded into Assets when it is selected
        return Assets::TexIDFromPath(_selectedFrame->label);
    }

    inline ModelID GetPickedShape() const
    {
        assert(_mode == Mode::SHAPES);
        if (!_selectedFrame) return NO_MODEL;
        return _selectedFrame->shape;
    }

protected:
    //Retrieves files, recursively, and generates frames for each.
    void _GetFrames(std::string rootDir);

    void _DrawGridView(Rectangle framesView);
    void _DrawListView(Rectangle framesView);
    void _DrawFrame(Frame *frame, Rectangle rect);

    std::vector<Frame> _frames;
    std::vector<Frame *> _filteredFrames;
    Frame *_selectedFrame;
    size_t _longestLabelLength;
    
    char _searchFilterBuffer[SEARCH_BUFFER_SIZE];
    bool _searchFilterFocused;

    Mode _mode;
    View _view;
    Vector2 _scroll;
};

#endif