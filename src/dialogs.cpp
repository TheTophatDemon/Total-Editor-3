#include "dialogs.hpp"

#include "raylib.h"
#include "extras/raygui.h"

#include <vector>
#include <initializer_list>
#include <cstring>

#include "math_stuff.hpp"
#include "app.hpp"
#include "map_man.hpp"

Rectangle DialogRec(float w, float h)
{
    return CenteredRect((float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f, w, h);
}

//Takes the given rectangles and returns a vector of new rectangles of the same sizes, that are arranged evenly spaced in a horizontal strip along the `bounds` rectangle.
std::vector<Rectangle> ArrangeHorzCentered(Rectangle bounds, std::initializer_list<Rectangle> rects)
{
    const float boundsHH = bounds.height / 2.0f;

    float itemMargin = bounds.width;
    for (const Rectangle &r : rects) itemMargin -= r.width;
    itemMargin /= rects.size();
    itemMargin /= 2.0f;

    std::vector<Rectangle> out = rects;
    float xofs = itemMargin;
    for (Rectangle &r : out)
    {
        r.x = bounds.x + xofs;
        r.y = bounds.y + boundsHH - (r.height / 2.0f);
        xofs += r.width + itemMargin;
    }

    return out;
}

NewMapDialog::NewMapDialog()
{
    const TileGrid &map = App::Get()->GetMapMan().Tiles();
    _width = map.GetWidth();
    _height = map.GetHeight();
    _length = map.GetLength();

    for (int i = 0; i < NUM_SPINNERS; ++i) _spinnerActive[i] = false;
}

bool NewMapDialog::Draw()
{
    static const float DW = 512.0f;
    static const float DHW = DW / 2.0f;
    static const float DH = 160.0f;
    static const float DHH = DH / 2.0f;
    static const float MARGIN = 8.0f;
    static const float SPINNER_H = 32.0f;
    static const float SPINNER_HH = SPINNER_H / 2.0f;
    static const float SPINNER_SEP = 32.0f;
    static const float SPINNER_W = (DW - (MARGIN * 2.0f) - (SPINNER_SEP * 2.0f)) / 3.0f;
    static const char *SPINNER_LABELS[NUM_SPINNERS] = { "Width", "Height", "Length" };
    
    int *spinnerPtrs[NUM_SPINNERS] = { &_width, &_height, &_length };

    Rectangle dRect = DialogRec(DW, DH);

    bool clicked = GuiWindowBox(dRect, "Enter dimensions for new map");

    //Size spinners
    for (int i = 0; i < NUM_SPINNERS; ++i)
    {
        Rectangle spRec = (Rectangle) { 
            .x = dRect.x + MARGIN + i * (SPINNER_W + SPINNER_SEP), 
            .y = dRect.y + DHH - SPINNER_HH, 
            .width = SPINNER_W, 
            .height = SPINNER_H
        };
        if (GuiSpinner(spRec, "", spinnerPtrs[i], 1, 1000, _spinnerActive[i]))
        {
            _spinnerActive[i] = !_spinnerActive[i];
        }
        GuiLabel((Rectangle) { .x = spRec.x, .y = spRec.y - MARGIN }, SPINNER_LABELS[i]);
    }

    //Confirm button
    if (GuiButton(CenteredRect(dRect.x + DHW, dRect.y + DH - MARGIN - SPINNER_HH, 128.0f, SPINNER_H), "CREATE"))
    {
        App::Get()->NewMap(_width, _height, _length);
        return false;
    }

    return !clicked;
}

ExpandMapDialog::ExpandMapDialog()
    : _spinnerActive(false),
      _chooserActive(false),
      _amount(0),
      _direction(Direction::Z_NEG)
{
}

bool ExpandMapDialog::Draw()
{
    static const float DW = 512.0f;
    static const float DHW = DW / 2.0f;
    static const float DH = 160.0f;
    static const float DHH = DH / 2.0f;
    static const float MARGIN = 8.0f;
    static const float SPINNER_W = 96.0f;
    static const float SPINNER_H = 32.0f;
    static const float SPINNER_HH = SPINNER_H / 2.0f;
    static const std::string SPINNER_LABEL = "# of grid cels to expand";

    Rectangle dRect = DialogRec(DW, DH);

    bool clicked = GuiWindowBox(dRect, "Expand map grid");
    //Confirm button
    if (GuiButton(CenteredRect(dRect.x + DHW, dRect.y + DH - MARGIN - SPINNER_HH, 128.0f, SPINNER_H), "EXPAND"))
    {
        App::Get()->ExpandMap(_direction, _amount);
        return false;
    }

    std::vector<Rectangle> recs = ArrangeHorzCentered(dRect, {
        (Rectangle) { .width = 128.0f, .height = 32.0f }, //Direction chooser
        (Rectangle) { .width = 96.0f, .height = 32.0f } //Amount spinner
    });

    //Direction chooser
    if (GuiDropdownBox(recs[0], "Back (+Z);Front (-Z);Right (+X);Left (-X);Top (+Y);Bottom (-Y)", (int *)&_direction, _chooserActive))
    {
        _chooserActive = !_chooserActive;
    }
    GuiLabel((Rectangle) { .x = recs[0].x, .y = recs[0].y - MARGIN }, "Direction");

    //Amount spinner
    if (GuiSpinner(recs[1], "", &_amount, 1, 1000, _spinnerActive))
    {
        _spinnerActive = !_spinnerActive;
    }
    GuiLabel((Rectangle) { .x = recs[1].x, .y = recs[1].y - MARGIN }, "# of cels");


    return !clicked;
}

bool ShrinkMapDialog::Draw()
{
    int choice = GuiMessageBox(DialogRec(352.0f, 96.0f), "Shrink Grid", "Shrink the grid to fit around the tiles?", "Yes;No");
    switch (choice)
    {
        case 0:
        case 2: 
            return false; 
        case 1: 
            App::Get()->ShrinkMap();
            return false;
    }
    return true;
}

EditEntDialog::EditEntDialog()
    : EditEntDialog((Ent) {
        .model = nullptr,
        .sprite = nullptr,
        .color = WHITE
      })
{
}

EditEntDialog::EditEntDialog(Ent ent)
    : _type(Type::EMPTY),
      _typeChooserActive(false),
      _propsScroll(Vector2Zero()),
      _ent(ent),
      _keyNameEdit(false)
{
    memset(&_filePath, 0, TEXT_FIELD_MAX * sizeof(char));
    memset(&_keyName, 0, TEXT_FIELD_MAX * sizeof(char));

    _ent.properties["name"] = "entity";
    _ent.properties["type"] = "wraith";
    _ent.properties["chill factor"] = "0.9";
    _ent.properties["favorite direction"] = "(1.0, 0.0, 0.0)";
    _ent.properties["least favorite regexp"] = "s/[Aa]re\\s[Yy]ou\\s\\(.*\\)?/Indeed, I am \\1./";
    _ent.properties["hingle"] = "12";
    _ent.properties["dingle"] = "";
    _ent.properties["shingle"] = "?";

    for (auto [key, value] : _ent.properties)
    {
        _propEditing[key] = false;
        _propBuffers[key] = (char *)malloc(sizeof(char) * TEXT_FIELD_MAX);
        strcpy(_propBuffers[key], _ent.properties[key].c_str());
    }
}

EditEntDialog::~EditEntDialog()
{
    for (auto [k, v] : _propBuffers)
    {
        free(v);
    }
}

bool EditEntDialog::Draw()
{
    const Rectangle DRECT = DialogRec(GetScreenWidth(), GetScreenHeight());

    bool clicked = GuiWindowBox(DRECT, "Edit Entity");

    const Rectangle TYPE_CHOOSER_RECT = (Rectangle) { 
        .x = DRECT.x + 128.0f, 
        .y = DRECT.y + 32.0f, 
        .width = DRECT.width - 8.0f - (TYPE_CHOOSER_RECT.x - DRECT.x), 
        .height = 32.0f 
    };
    
    const Rectangle APPEAR_RECT = (Rectangle) {
        DRECT.x + 8.0f, TYPE_CHOOSER_RECT.y + TYPE_CHOOSER_RECT.height + 8.0f, DRECT.width - 16.0f, 128
    };

    //Appearance configuration
    if (_type == Type::EMPTY)
    {
        //Color picker for empty entities.
        Color newColor = GuiColorPicker((Rectangle) { APPEAR_RECT.x, APPEAR_RECT.y, APPEAR_RECT.width - 32.0f, APPEAR_RECT.height }, "Color", _ent.color);
        if (!_typeChooserActive) _ent.color = newColor;
    }
    else
    {
        //File selector for other types.

    }

    //Property list
    Rectangle SCROLL_RECT = (Rectangle) {
        DRECT.x + 8.0f, APPEAR_RECT.y + APPEAR_RECT.height + 8.0f, DRECT.width - 16.0f, DRECT.height - 8.0f - (SCROLL_RECT.y - DRECT.y) - 64.0f - 4.0f
    };
    const float PROP_HEIGHT = 24.0f;

    //The contents of the list get expanded horizontally according to the longest value string
    int longestStringLen = 0;
    for (const auto &[k, v] : _ent.properties)
    {
        longestStringLen = Max(longestStringLen, v.length() + 1);
    }

    Rectangle scissor = GuiScrollPanel(SCROLL_RECT, "Properties", 
        (Rectangle) { .width = Max((int)SCROLL_RECT.width - 16, DRECT.width / 2 + longestStringLen * 12), .height = (PROP_HEIGHT * _ent.properties.size()) + 8.0f }, 
        &_propsScroll);
    
    BeginScissorMode(scissor.x, scissor.y, scissor.width, scissor.height);

    float y = 4.0f;
    for (auto [key, val] : _ent.properties)
    {
        const Rectangle KEY_RECT = (Rectangle) { scissor.x + 4.0f + _propsScroll.x, scissor.y + y + _propsScroll.y, (scissor.width - 16.0f) / 2.0f, PROP_HEIGHT };
        if (GuiLabelButton(KEY_RECT, key.c_str()))
        {
            //Copy key name field when a key is clicked in the list.
            strcpy(_keyName, key.c_str());
        }
        const Rectangle VAL_RECT = (Rectangle) { KEY_RECT.x + KEY_RECT.width + 4.0f, KEY_RECT.y, Max(KEY_RECT.width, longestStringLen * 12), PROP_HEIGHT };
        if (GuiTextBox(VAL_RECT, _propBuffers[key], TEXT_FIELD_MAX, _propEditing[key]))
        {
            _propEditing[key] = !_propEditing[key];
        }
        _ent.properties[key] = std::string(_propBuffers[key]);
        y += PROP_HEIGHT;
    }

    EndScissorMode();

    //Key add/remove widgets
    const Rectangle ADD_KEY_RECT = (Rectangle) { SCROLL_RECT.x, SCROLL_RECT.y + SCROLL_RECT.height, 32, 32 };
    const Rectangle REM_KEY_RECT = (Rectangle) { ADD_KEY_RECT.x + ADD_KEY_RECT.width + 4.0f, ADD_KEY_RECT.y, 32, 32 };
    if (GuiButton(ADD_KEY_RECT, "+"))
    {
        //Add key
        if (strlen(_keyName) > 0 && _ent.properties.find(_keyName) == _ent.properties.end())
        {
            _ent.properties[_keyName] = "";
            _propBuffers[_keyName] = (char *)malloc(sizeof(char) * TEXT_FIELD_MAX);
            memset(_propBuffers[_keyName], 0, sizeof(char) * TEXT_FIELD_MAX);
            _propEditing[_keyName] = false;
        }
    }
    else if (GuiButton(REM_KEY_RECT, "-"))
    {
        if (strlen(_keyName) > 0 && _ent.properties.find(_keyName) != _ent.properties.end())
        {
            _ent.properties.erase(_keyName);
            free(_propBuffers[_keyName]);
            _propBuffers.erase(_keyName);
            _propEditing.erase(_keyName);
        }
    }
    //Key name input field
    const Rectangle KEY_NAME_RECT = (Rectangle) { REM_KEY_RECT.x + REM_KEY_RECT.width + 4.0f, REM_KEY_RECT.y, DRECT.width - (KEY_NAME_RECT.x - DRECT.x) - 8.0f, 32.0f };
    if (GuiTextBox(KEY_NAME_RECT, _keyName, TEXT_FIELD_MAX, _keyNameEdit))
    {
        _keyNameEdit = !_keyNameEdit;
    }

    //Confirm button
    std::vector<Rectangle> recs = ArrangeHorzCentered((Rectangle) { DRECT.x, DRECT.y + DRECT.height - 32.0f - 6.0f, DRECT.width, 32.0f }, 
        { (Rectangle) { .width = 128.0f, .height = 32.0f } });

    if (GuiButton(recs[0], "Place"))
    {
        //TODO: Put entity
        return false;
    }

    //Type chooser
    GuiLabel((Rectangle) { DRECT.x + 4.0f, TYPE_CHOOSER_RECT.y + 16.0f }, "Display-Type");
    if (GuiDropdownBox(TYPE_CHOOSER_RECT, "Empty;Sprite;Model", (int *)&_type, _typeChooserActive))
    {
        _typeChooserActive = !_typeChooserActive;
    }
    
    return !clicked;
}