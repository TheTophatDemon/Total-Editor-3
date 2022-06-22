#ifndef DIALOGS_H
#define DIALOGS_H

#include <map>

#include "tile.hpp"
#include "ent.hpp"

#define TEXT_FIELD_MAX 512

class Dialog 
{ 
public:
    inline virtual ~Dialog() {}
    //Returns false when the dialog should be closed.
    virtual bool Draw() = 0; 
};

class NewMapDialog : public Dialog 
{
public: 
    NewMapDialog();
    NewMapDialog(int width, int height, int length);
    virtual bool Draw() override;
protected:
    inline static constexpr int NUM_SPINNERS = 3;
    int _width;
    int _height;
    int _length;
    bool _spinnerActive[NUM_SPINNERS];
};

class ExpandMapDialog: public Dialog
{
public:
    ExpandMapDialog();
    virtual bool Draw() override;
protected:
    bool _chooserActive;
    bool _spinnerActive;
    int _amount;
    Direction _direction;
};

class ShrinkMapDialog: public Dialog
{
public:
    virtual bool Draw() override;
};

#endif