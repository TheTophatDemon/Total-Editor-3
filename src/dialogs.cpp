/**
 * Copyright (c) 2022 Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "dialogs.hpp"

#include "raylib.h"
#include "extras/raygui.h"

#include <vector>
#include <initializer_list>
#include <map>
#include <iostream>

#include "assets.hpp"
#include "math_stuff.hpp"
#include "app.hpp"
#include "map_man.hpp"
#include "text_util.hpp"
#include "draw_extras.h"

Rectangle DialogRec(float w, float h)
{
    return CenteredRect((float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f, w, h);
}

//Takes the given rectangles and returns a vector of new rectangles of the same sizes, that are arranged evenly spaced in a horizontal strip along the `bounds` rectangle.
std::vector<Rectangle> ArrangeHorzCentered(Rectangle bounds, std::initializer_list<Rectangle> rects)
{
    const float boundsHH = bounds.height / 2.0f;

    const float OFFSET = bounds.width / (rects.size() + 1);
    std::vector<Rectangle> out = rects;
    float xCenter = OFFSET;
    for (Rectangle &r : out)
    {
        r.x = bounds.x + xCenter - (r.width / 2.0f);
        r.y = bounds.y + boundsHH - (r.height / 2.0f);
        xCenter += OFFSET;
    }

    return out;
}

//Takes some rectangles and returns copies of them, repositioned to be vertically laid out inside of the `area` rectangle.
std::vector<Rectangle> ArrangeVertical(Rectangle area, std::initializer_list<Rectangle> rects)
{
    const float REGION_HEIGHT = area.height / rects.size();

    std::vector<Rectangle> out = rects;
    int i = 0;;
    for (Rectangle& r : out)
    {
        r.x += area.x;
        r.y += area.y + (i * REGION_HEIGHT) + (REGION_HEIGHT / 2.0f) - (r.height / 2.0f);
        ++i;
    }
    return out;
}

//Takes some rectangles, that are all the same size, and returns copies of them, repositioned to be vertically laid out inside of the `area` rectangle.
std::vector<Rectangle> ArrangeVertical(Rectangle area, Rectangle templ, int count)
{
    const float REGION_HEIGHT = area.height / count;

    std::vector<Rectangle> out(count);
    int i = 0;
    for (Rectangle& r : out)
    {
        r = templ;
        r.x += area.x;
        r.y += area.y + (i * REGION_HEIGHT) + (REGION_HEIGHT / 2.0f) - (r.height / 2.0f);
        ++i;
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
        Rectangle spRec = Rectangle { 
            .x = dRect.x + MARGIN + i * (SPINNER_W + SPINNER_SEP), 
            .y = dRect.y + DHH - SPINNER_HH, 
            .width = SPINNER_W, 
            .height = SPINNER_H
        };
        if (GuiSpinner(spRec, "", spinnerPtrs[i], 1, 1000, _spinnerActive[i]))
        {
            _spinnerActive[i] = !_spinnerActive[i];
        }
        GuiLabel(Rectangle { .x = spRec.x, .y = spRec.y - MARGIN }, SPINNER_LABELS[i]);
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
        Rectangle { .width = 128.0f, .height = 32.0f }, //Direction chooser
        Rectangle { .width = 96.0f, .height = 32.0f } //Amount spinner
    });

    //Direction chooser
    if (GuiDropdownBox(recs[0], "Back (+Z);Front (-Z);Right (+X);Left (-X);Top (+Y);Bottom (-Y)", (int *)&_direction, _chooserActive))
    {
        _chooserActive = !_chooserActive;
    }
    GuiLabel(Rectangle { .x = recs[0].x, .y = recs[0].y - MARGIN }, "Direction");

    //Amount spinner
    if (GuiSpinner(recs[1], "", &_amount, 1, 1000, _spinnerActive))
    {
        _spinnerActive = !_spinnerActive;
    }
    GuiLabel(Rectangle { .x = recs[1].x, .y = recs[1].y - MARGIN }, "# of cels");


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

    const Rectangle FILE_ENTRY_RECT = Rectangle { 
        .x = DRECT.x + 4.0f, 
        .y = DRECT.y + DRECT.height - 24.0f, 
        .width = DRECT.width - 64.0f - 8.0f, 
        .height = 20.0f
    };

    const Rectangle FILES_RECT = Rectangle {
        .x = DRECT.x + 4.0f,
        .y = DRECT.y + 28.0f,
        .width = DRECT.width - 8.0f,
        .height = DRECT.height - (FILES_RECT.y - DRECT.y) - FILE_ENTRY_RECT.height - 8.0f
    };

    //Collect file information to display on the screen
    Rectangle content = Rectangle { .width = FILES_RECT.width - 8.0f, .height = 8.0f };
    std::map<const fs::directory_entry, Rectangle> fileRects;

    const float FILE_RECT_HEIGHT = 16.0f;

    float y = 4.0f;
    //Add the parent directory to the list of files as [parent folder]
    if (_currentDir.has_parent_path())
    {
        fileRects[fs::directory_entry(_currentDir.parent_path())] = Rectangle { 4.0f, y, content.width - 8.0f, FILE_RECT_HEIGHT };
        y += FILE_RECT_HEIGHT;
        content.height += FILE_RECT_HEIGHT;
    }

    //Calculate rectangles and total height for all the files
    for (auto const& entry : fs::directory_iterator{_currentDir})
    {
        if (entry.is_directory() || 
            (entry.is_regular_file() && _extensions.find(entry.path().extension().string()) != _extensions.end()))
        {
            Rectangle fileRect = Rectangle {
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
    BeginScissorMode(scissor.x, scissor.y, int(scissor.width), int(scissor.height));

    for (auto const& [entry, rect] : fileRects)
    {
        std::string name;
        if (_currentDir.has_parent_path() && entry.path() == _currentDir.parent_path())
        {
            name = "[parent directory]";
        }
        else
        {
            name = entry.path().filename().string();
        }

        const Rectangle BUTT_RECT = Rectangle { 
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
                strcpy(_fileNameBuffer, entry.path().filename().string().c_str());
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
    const Rectangle SELECT_BUTT_RECT = Rectangle {
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

static const int N_QUIT_MESSAGES = 9;
static const char *QUIT_MESSAGES[N_QUIT_MESSAGES] = {
    "Only winners take stretch breaks.", 
    "Did I leave the editor on Nightmare difficulty?", 
    "Remember to eat some clowns.", 
    "Admiring the *cough* robust C++ architecture?", 
    "Your SOUL is what needs saving, not this map file!",
    "Click 'Nah' to meet hot singles in your area!",
    "Whelp...I'm going to Grillby's...",
    "100% of people who go outside die!",
    "Да, я тоже не понимаю это приложение."
};

CloseDialog::CloseDialog()
{
    _messageIdx = GetRandomValue(0, N_QUIT_MESSAGES - 1);
}

bool CloseDialog::Draw()
{
    switch (GuiMessageBox(DialogRec(480.0f, 160.0f), "Really quit?", QUIT_MESSAGES[_messageIdx], "Quit;Nah"))
    {
    case 1:
        App::Get()->Quit();
        return false;
    case -1:
        return true;
    default:
        return false;
    }
}

AssetPathDialog::AssetPathDialog(App::Settings &settings)
    : _settings(settings),
      _texPathEdit(false),
      _shapePathEdit(false)
{
    strcpy(_texPathBuffer, App::Get()->GetTexturesDir().c_str());
    strcpy(_shapePathBuffer, App::Get()->GetShapesDir().c_str());
}

bool AssetPathDialog::Draw()
{
    const Rectangle DRECT = DialogRec(632.0f, 256.0f);
    
    bool clicked = GuiWindowBox(DRECT, "Asset Paths");

    const Rectangle TEX_PATH_BOX = Rectangle {
        .x = DRECT.x + 8.0f,
        .y = DRECT.y + 8.0f + 32.0f + 12.0f,
        .width = DRECT.width - (TEX_PATH_BOX.x - DRECT.x) - 16.0f,
        .height = 24.0f
    };

    GuiLabel(Rectangle{ TEX_PATH_BOX.x, TEX_PATH_BOX.y - 12.0f }, "Textures Path");
    if (GuiTextBox(TEX_PATH_BOX, _texPathBuffer, TEXT_FIELD_MAX, _texPathEdit))
    {
        _texPathEdit = !_texPathEdit;
    }

    const Rectangle SHAPE_PATH_BOX = Rectangle {
        .x = TEX_PATH_BOX.x,
        .y = TEX_PATH_BOX.y + TEX_PATH_BOX.height + 24.0f,
        .width = TEX_PATH_BOX.width,
        .height = TEX_PATH_BOX.height
    };
    GuiLabel(Rectangle { SHAPE_PATH_BOX.x, SHAPE_PATH_BOX.y - 12.0f }, "Shapes Path");
    if (GuiTextBox(SHAPE_PATH_BOX, _shapePathBuffer, TEXT_FIELD_MAX, _shapePathEdit))
    {
        _shapePathEdit = !_shapePathEdit;
    }

    const Rectangle BUTTON_REGION = Rectangle {
        .x = DRECT.x + 8.0f,
        .y = DRECT.y + DRECT.height - 48.0f,
        .width = DRECT.width - (BUTTON_REGION.x - DRECT.x) - 8.0f,
        .height = 48.0f
    };
    std::vector<Rectangle> buttonRecs = ArrangeHorzCentered(BUTTON_REGION, { 
        Rectangle{ .width = 96.0f, .height = 32.0f }, 
        Rectangle{ .width = 96.0f, .height = 32.0f } });

    if (GuiButton(buttonRecs[0], "Confirm"))
    {
        bool bad = false;
        fs::directory_entry texEntry { _texPathBuffer };
        if (!texEntry.exists() || !texEntry.is_directory())
        {
            strcpy(_texPathBuffer, "Invalid!");
            bad = true;
        }
        fs::directory_entry shapeEntry { _shapePathBuffer };
        if (!shapeEntry.exists() || !shapeEntry.is_directory())
        {
            strcpy(_shapePathBuffer, "Invalid!");
            bad = true;
        }
        if (!bad) 
        {
            _settings.texturesDir = texEntry.path().string();
            _settings.shapesDir = shapeEntry.path().string();
            App::Get()->SaveSettings();
            return false;
        }
    }
    else if (GuiButton(buttonRecs[1], "Cancel"))
    {
        return false;
    }

    return !clicked;
}

SettingsDialog::SettingsDialog(App::Settings &settings)
    : _settings(settings),
      _undoMaxEdit(false),
      _undoMax(settings.undoMax),
      _sensitivity(settings.mouseSensitivity)
{
}

bool SettingsDialog::Draw()
{
    const Rectangle DRECT = DialogRec(512.0f, 256.0f);

    bool clicked = GuiWindowBox(DRECT, "Settings");

    const Rectangle SETTINGS_RECT = Rectangle { DRECT.x + 8.0f, DRECT.y + 32.0f, DRECT.width - 16.0f, DRECT.height - 64.0f };
    std::vector<Rectangle> recs = ArrangeVertical(
        SETTINGS_RECT,
        {
            Rectangle { .x = 16.0f, .width = 128.0f, .height = 32.0f }, //0: Undo max
            Rectangle { .x = 16.0f, .width = SETTINGS_RECT.width - 64.0f, .height = 32.0f }  //1: Sensitivity
        }
    );

    GuiLabel(Rectangle { recs[0].x, recs[0].y - 12.0f }, "Max Undo Count");
    if (GuiSpinner(recs[0], "", &_undoMax, 1, 1000, _undoMaxEdit))
    {
        _undoMaxEdit = !_undoMaxEdit;
    }

    GuiLabel(Rectangle { recs[1].x, recs[1].y - 12.0f }, TextFormat("Mouse sensitivity: %.2f", _sensitivity));
    _sensitivity = GuiSlider(recs[1], "", "", _sensitivity, 0.05f, 10.0f);
    _sensitivity = floorf(_sensitivity / 0.05f) * 0.05f;

    //Confirm buttons
    const Rectangle BUTT_GROUP = Rectangle { DRECT.x + 8.0f, DRECT.y + DRECT.height - 8.0f - 32.0f, DRECT.width - 16.0f, 32.0f };
    std::vector<Rectangle> buttRecs = ArrangeHorzCentered(BUTT_GROUP, {
        Rectangle { .width = 128.0f, .height = 32.0f }, //0: Confirm
        Rectangle { .width = 128.0f, .height = 32.0f }  //1: Cancel
    });

    if (GuiButton(buttRecs[0], "Confirm"))
    {
        _settings.undoMax = _undoMax;
        _settings.mouseSensitivity = _sensitivity;
        App::Get()->SaveSettings();
        return false;
    }

    if (GuiButton(buttRecs[1], "Cancel"))
    {
        return false;
    }

    return !clicked;
}

bool AboutDialog::Draw()
{
    const Rectangle DRECT = DialogRec(480.0f, 256.0f);

    bool clicked = GuiWindowBox(DRECT, "About");

    Font font = Assets::GetFont();
    DrawTextEx(font, "Total Editor 3.0.0", Vector2 { DRECT.x + 8.0f, DRECT.y + 32.0f }, 32.0f, 0.25f, WHITE);

    DrawTextEx(font, "Written by The Tophat Demon", Vector2 { DRECT.x + 8.0f, DRECT.y + 70.0f }, font.baseSize, 0.0f, WHITE);
    DrawTextEx(font, "Source code: \nhttps://github.com/TheTophatDemon/Total-Editor-3", Vector2 { DRECT.x + 8.0f, DRECT.y + 96.0f }, font.baseSize, 0.0f, WHITE);

    return !clicked;
}

ShortcutsDialog::ShortcutsDialog()
    : _scroll(Vector2Zero())
{
}

bool ShortcutsDialog::Draw()
{
    static const int N_SHORTCUTS = 26;
    static const char *SHORTCUTS_TEXT[N_SHORTCUTS] = {
        "W/A/S/D - Move camera",
        "Middle click - Look around",
        "Scroll wheel - Move grid up/down",
        "Left click - Place tile/entity/brush",
        "Right click - Remove tile/entity (Does not work in brush mode)",
        "TAB - Switch between texture picker and map editor.",
        "LEFT SHIFT+TAB - Switch between shape picker and map editor.",
        "T (Tile mode) - Select texture of tile under cursor",
        "G (Tile mode) - Select shape of tile under cursor",
        "HOLD LEFT SHIFT - Expand cursor to place tiles in bulk.",
        "Q - Turn cursor counterclockwise",
        "E - Turn cursor clockwise",
        "R - Reset cursor orientation",
        "F - Turn cursor upwards",
        "V - Turn cursor downwards",
        "H - Isolate the layer of tiles the grid is on.",
        "H (when layers are isolated) - Unhide hidden layers.",
        "Hold H while using scrollwheel - Select multiple layers to isolate.",
        "LEFT SHIFT+B - Capture tiles under cursor as a brush.",
        "ESCAPE/BACKSPACE - Return cursor to tile mode.",
        "LEFT CTRL+TAB - Switch between entity editor and map editor.",
        "LEFT CTRL+E - Put cursor into entity mode.",
        "T/G (Entity mode) - Copy entity from under cursor.",
        "LEFT CTRL+S - Save map.",
        "LEFT CTRL+Z - Undo",
        "LEFT CTRL+Y - Redo"
    };
    
    const Rectangle DRECT = DialogRec(608.0f, 468.0f);

    bool clicked = GuiWindowBox(DRECT, "Shortcuts");

    Rectangle contentRect = Rectangle { 0 };
    const float TEXT_HEIGHT = 32.0f;
    contentRect.height = 16.0f;
    for (int i = 0; i < N_SHORTCUTS; ++i)
    {
        std::string str = SHORTCUTS_TEXT[i];
        float w = (float)GetStringWidth(Assets::GetFont(), (float)GuiGetStyle(DEFAULT, TEXT_SIZE), str);
        if (contentRect.width < w) contentRect.width = w;
        contentRect.height += TEXT_HEIGHT;
    }
    contentRect.width += 100.0f;

    Rectangle scissor = GuiScrollPanel(
        Rectangle { DRECT.x + 8.0f, DRECT.y + 32.0f, DRECT.width - 16.0f, DRECT.height - 40.0f }, 
        NULL, contentRect, &_scroll);

    BeginScissorMode((int)scissor.x, (int)scissor.y, (int)scissor.width, (int)scissor.height);

    for (int i = 0; i < N_SHORTCUTS; ++i)
    {
        GuiLabel(Rectangle { scissor.x + _scroll.x, scissor.y + _scroll.y + 16.0f + (i * TEXT_HEIGHT) }, SHORTCUTS_TEXT[i]);
    }

    EndScissorMode();

    return !clicked;
}

bool InstructionsDialog::Draw()
{
    const Rectangle DRECT = DialogRec(512.0f, 128.0f);

    bool clicked = GuiWindowBox(DRECT, "Instructions");

    GuiLabel(Rectangle{ .x = DRECT.x + 8.0f, .y = DRECT.y + 64.0f }, "Please refer to the file \n'instructions.html' included with the application.");

    return !clicked;
}

ExportDialog::ExportDialog(App::Settings &settings)
    : _settings(settings),
      _filePathEdit(false)
{
    strcpy(_filePathBuffer, _settings.exportFilePath.c_str());
}

bool ExportDialog::Draw()
{
    const Rectangle DRECT = DialogRec(512.0f, 256.0f);

    if (_dialog.get())
    {
        GuiLock();
    }

    bool clicked = GuiWindowBox(DRECT, "Export .gltf scene");

    const Rectangle FILEPATH_RECT = Rectangle { DRECT.x + 8.0f, DRECT.y + 48.0f, DRECT.width - 16.0f, 24.0f };
    strcpy(_filePathBuffer, _settings.exportFilePath.c_str());
    if (GuiTextBox(FILEPATH_RECT, _filePathBuffer, TEXT_FIELD_MAX, _filePathEdit))
    {
        _filePathEdit = !_filePathEdit;
    }   
    _settings.exportFilePath = _filePathBuffer;
    GuiLabel(Rectangle { .x = FILEPATH_RECT.x, .y = FILEPATH_RECT.y - 12.0f }, "File path");

    const Rectangle BROWSE_BUTT_RECT = Rectangle { FILEPATH_RECT.x, FILEPATH_RECT.y + FILEPATH_RECT.height + 4.0f, 128.0f, 32.0f };
    if (GuiButton(BROWSE_BUTT_RECT, "Browse"))
    {
        _dialog.reset(new FileDialog(std::string("Save .GLTF file"), {std::string(".gltf")}, [&](fs::path path){
            _settings.exportFilePath = fs::relative(path).string();
        }));
    }

    const Rectangle SEP_BUTT_RECT = Rectangle { BROWSE_BUTT_RECT.x, BROWSE_BUTT_RECT.y + BROWSE_BUTT_RECT.height + 32.0f, 32.0f, 32.0f };
    _settings.exportSeparateGeometry = GuiCheckBox(SEP_BUTT_RECT, "Separate nodes for each texture", _settings.exportSeparateGeometry);

    const Rectangle EXPORT_BUTT_RECT = Rectangle { DRECT.x + DRECT.width / 2.0f - 64.0f, DRECT.y + DRECT.height - 40.0f, 128.0f, 32.0f };
    if (GuiButton(EXPORT_BUTT_RECT, "Export"))
    {
        App::Get()->TryExportMap(fs::path(_settings.exportFilePath), _settings.exportSeparateGeometry);
        App::Get()->SaveSettings();
        return false;
    }

    if (_dialog.get() && GuiIsLocked())
    {
        GuiUnlock();
        if (!_dialog->Draw())
        {
            _dialog.reset();
        }
    }

    if (clicked) App::Get()->SaveSettings();

    return !clicked;
}
