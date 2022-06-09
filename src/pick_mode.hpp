#ifndef PICK_MODE_H
#define PICK_MODE_H

#include "raylib.h"

#include <vector>
#include <string>

#include "editor_mode.hpp"
#include "app.hpp"

class PickMode : public EditorMode {
public:
    struct Frame {
        Texture2D *tex;
        std::string label;
    };

    enum class Mode { TEXTURES, SHAPES };

    PickMode(AppContext *context, Mode mode);
    virtual void Update() override;
    virtual void Draw() override;
protected:
    AppContext *_context;
    std::vector<Frame> _frames;
    Frame *_selectedFrame;
    std::string _searchFilter;
    Rectangle _framesView;
    Rectangle _framesContent;
    Mode _mode;
    Vector2 _scroll;
};

#endif