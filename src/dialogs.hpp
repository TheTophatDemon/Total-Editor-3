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

class EditEntDialog : public Dialog
{
public:
    EditEntDialog();
    EditEntDialog(Ent ent);
    virtual ~EditEntDialog();
    virtual bool Draw() override;
protected:
    enum class Type { EMPTY, SPRITE, MODEL };
    Ent _ent;
    Type _type;
    Vector2 _propsScroll;
    //Indicates which properties are having their values edited.
    std::map<std::string, bool> _propEditing;
    std::map<std::string, char*> _propBuffers; //In order to work with RayGUI text boxes, each value needs a modifiable char buffer.
    char _keyName[TEXT_FIELD_MAX];
    bool _keyNameEdit;
    char _filePath[TEXT_FIELD_MAX];
    bool _typeChooserActive;
};

#endif