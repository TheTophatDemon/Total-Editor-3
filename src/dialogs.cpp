#include "dialogs.hpp"

#include "raylib.h"
#include "extras/raygui.h"

#include <vector>
#include <initializer_list>

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
    const TileGrid &map = App::Get()->GetMapMan().Map();
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
    if (GuiDropdownBox(recs[0], "Back (+Z);Front (-Z);Right (+X);Left (-X);TOP (+Y);BOTTOM(-Y)", (int *)&_direction, _chooserActive))
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