#include "dialogs.hpp"

#include "raylib.h"
#include "extras/raygui.h"

#include "math_stuff.hpp"
#include "app.hpp"
#include "map_man.hpp"

Rectangle DialogRec(float w, float h)
{
    return CenteredRect((float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f, w, h);
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