#ifndef EDITOR_MODE_H
#define EDITOR_MODE_H

class EditorMode {
public:
    virtual void Update() = 0;
    virtual void Draw() = 0;
};

#endif