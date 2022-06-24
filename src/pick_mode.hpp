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
        TexID tex;
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
        return _selectedFrame->tex;
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