#ifndef PICK_MODE_H
#define PICK_MODE_H

#include "raylib.h"

#include <vector>

#include "editor_mode.hpp"

class PickMode : public EditorMode {
public:
    PickMode();
    virtual void Update() override;
    virtual void Draw() override;
protected:
    
};

#endif