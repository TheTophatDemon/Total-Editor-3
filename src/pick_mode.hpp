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

    PickMode(AppContext *context, Mode mode);
    virtual ~PickMode();
    virtual void Update() override;
    virtual void Draw() override;
    virtual void OnEnter() override;
    virtual void OnExit() override;
    
    // inline void SetMode(Mode mode) { _mode = mode; }
    inline Mode GetMode() const { return _mode; }
protected:
    //Retrieves files, recursively, and generates frames for each.
    void _GetFrames(std::string rootDir);

    AppContext *_context;
    std::vector<Frame> _frames;
    std::vector<Frame> _filteredFrames;
    Frame *_selectedFrame;
    
    char *_searchFilterBuffer;
    bool _searchFilterFocused;

    Mode _mode;
    Vector2 _scroll;
};

#endif