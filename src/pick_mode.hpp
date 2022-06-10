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
        Model *shape;
        std::string label;
    };

    enum class Mode { TEXTURES, SHAPES };
    enum class View { GRID, LIST };

    PickMode(AppContext *context, Mode mode);
    virtual ~PickMode();
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    
    // inline void SetMode(Mode mode) { _mode = mode; }
    inline Mode GetMode() const { return _mode; }
    inline View GetView() const { return _view; }
protected:
    //Retrieves files, recursively, and generates frames for each.
    void _GetFrames(std::string rootDir);

    void _DrawGridView(Rectangle framesView);
    void _DrawListView(Rectangle framesView);
    void _DrawFrame(Frame *frame, Rectangle rect);

    AppContext *_context;
    std::vector<Frame> _frames;
    std::vector<Frame *> _filteredFrames;
    Frame *_selectedFrame;
    size_t _longestLabelLength;
    
    char *_searchFilterBuffer;
    bool _searchFilterFocused;

    Mode _mode;
    View _view;
    Vector2 _scroll;
};

#endif