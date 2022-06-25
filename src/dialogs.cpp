#include "dialogs.hpp"

#include "raylib.h"
#include "extras/raygui.h"

#include <vector>
#include <initializer_list>
#include <map>

#include "assets.hpp"
#include "math_stuff.hpp"
#include "app.hpp"
#include "map_man.hpp"
#include "draw_extras.h"

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

bool FileDialog::Draw()
{
    const Rectangle DRECT = DialogRec(512.0f, 400.0f);
    bool clicked = GuiWindowBox(DRECT, _title.c_str());

    const Rectangle FILE_ENTRY_RECT = (Rectangle) { 
        .x = DRECT.x + 4.0f, 
        .y = DRECT.y + DRECT.height - 24.0f, 
        .width = DRECT.width - 64.0f - 8.0f, 
        .height = 20.0f
    };

    const Rectangle FILES_RECT = (Rectangle) {
        .x = DRECT.x + 4.0f,
        .y = DRECT.y + 28.0f,
        .width = DRECT.width - 8.0f,
        .height = DRECT.height - (FILES_RECT.y - DRECT.y) - FILE_ENTRY_RECT.height - 8.0f
    };

    //Collect file information to display on the screen
    Rectangle content = (Rectangle) { .width = FILES_RECT.width - 8.0f, .height = 8.0f };
    std::map<const fs::directory_entry, Rectangle> fileRects;

    const float FILE_RECT_HEIGHT = 16.0f;

    float y = 4.0f;
    //Add the parent directory to the list of files as [parent folder]
    if (_currentDir.has_parent_path())
    {
        fileRects[fs::directory_entry(_currentDir.parent_path())] = (Rectangle) { 4.0f, y, content.width - 8.0f, FILE_RECT_HEIGHT };
        y += FILE_RECT_HEIGHT;
        content.height += FILE_RECT_HEIGHT;
    }

    //Calculate rectangles and total height for all the files
    for (auto const& entry : fs::directory_iterator{_currentDir})
    {
        if (entry.is_directory() || 
            (entry.is_regular_file() && _extensions.find(entry.path().extension()) != _extensions.end()))
        {
            Rectangle fileRect = (Rectangle) {
                .x = 4.0f,
                .y = y,
                .width = content.width - 8.0f,
                .height = FILE_RECT_HEIGHT,
            };
            fileRects[entry] = fileRect;
            y += fileRect.height;
            content.height += fileRect.height;
        }
    }

    //Directory view drawing
    Rectangle scissor = GuiScrollPanel(FILES_RECT, NULL, content, &_scroll);
    BeginScissorMode(scissor.x, scissor.y, scissor.width, scissor.height);

    for (auto const& [entry, rect] : fileRects)
    {
        std::string name;
        if (_currentDir.has_parent_path() && entry.path() == _currentDir.parent_path())
        {
            name = "[parent directory]";
        }
        else
        {
            name = entry.path().filename();
        }

        const Rectangle BUTT_RECT = (Rectangle) { 
            .x = scissor.x + rect.x + _scroll.x, 
            .y = scissor.y + rect.y + _scroll.y, 
            .width = rect.width, 
            .height = rect.height 
        };

        if (GuiLabelButton(BUTT_RECT, name.c_str()))
        {
            if (entry.is_directory()) 
            {
                _currentDir = entry.path();
                memset(_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
            }
            else 
            {
                strcpy(_fileNameBuffer, entry.path().filename().c_str());
            }
        }
    }

    EndScissorMode();

    //File name entry

    if (GuiTextBox(FILE_ENTRY_RECT, _fileNameBuffer, TEXT_FIELD_MAX, _fileNameEdit))
    {
        _fileNameEdit = !_fileNameEdit;
    }

    //Select button
    const Rectangle SELECT_BUTT_RECT = (Rectangle) {
        .x = FILE_ENTRY_RECT.x + FILE_ENTRY_RECT.width + 4.0f,
        .y = FILE_ENTRY_RECT.y,
        .width = DRECT.width - (SELECT_BUTT_RECT.x - DRECT.x) - 4.0f,
        .height = FILE_ENTRY_RECT.height
    }; 

    if (GuiButton(SELECT_BUTT_RECT, "Select"))
    {
        if (strlen(_fileNameBuffer) > 0)
        {
            _callback(_currentDir.append(_fileNameBuffer));
            return false;
        }
    }

    return !clicked;
}