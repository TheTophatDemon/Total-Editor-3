/**
 * Copyright (c) 2022-present Alexander Lunsford
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
#include "imgui/imgui.h"

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
#include "gui.hpp"

Rectangle DialogRec(float w, float h)
{
    return CenteredRect((float)GetScreenWidth() / 2.0f, (float)GetScreenHeight() / 2.0f, w, h);
}

NewMapDialog::NewMapDialog()
{
    const TileGrid &map = App::Get()->GetMapMan().Tiles();
    _mapDims[0] = map.GetWidth();
    _mapDims[1] = map.GetHeight();
    _mapDims[2] = map.GetLength();
}

bool NewMapDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup("NEW MAP");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("NEW MAP", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("NEW GRID SIZE:");
        ImGui::InputInt3("X, Y, Z", _mapDims);
        
        if (ImGui::Button("CREATE"))
        {
            App::Get()->NewMap(_mapDims[0], _mapDims[1], _mapDims[2]);
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }
    return open;
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
    bool open = true;
    ImGui::OpenPopup("EXPAND GRID");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("EXPAND GRID", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Combo("Direction", (int*)(&_direction), "Back (+Z)\0Front (-Z)\0Right (+X)\0Left (-X)\0Top (+Y)\0Bottom (-Y)\0");

        ImGui::InputInt("# of grid cels", &_amount, 1, 10);
        
        if (ImGui::Button("EXPAND"))
        {
            App::Get()->ExpandMap(_direction, _amount);
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }
    return open;
}

bool ShrinkMapDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup("SHRINK GRID");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("SHRINK GRID", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted("Shrink the grid to fit around the existing tiles?");

        if (ImGui::Button("Yes"))
        {
            App::Get()->ShrinkMap();
            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("No"))
        {
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }
    return open;
}

FileDialog::FileDialog(std::string title, std::initializer_list<std::string> extensions, std::function<void(std::filesystem::path)> callback) 
    : _title(title),
        _extensions(extensions),
        _callback(callback),
        _currentDir(fs::current_path())
{
    memset(&_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
}

bool FileDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup(_title.c_str());
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(_title.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Parent directory button & other controls
        // if (!_currentDir.has_parent_path()) ImGui::BeginDisabled();
        if (ImGui::Button("Parent directory") && _currentDir.has_parent_path())
        {
            _currentDir = _currentDir.parent_path();
        }
        // if (!_currentDir.has_parent_path()) ImGui::EndDisabled();

        if (ImGui::BeginListBox("Files", ImVec2(504.0f, 350.0f)))
        {
            // File list
            fs::directory_iterator fileIter;
            try 
            {
                fileIter = fs::directory_iterator{_currentDir};
            }
            catch (...)
            {
                // For some reason, the directory iterator will show special system folders that aren't even accessible in the file explorer.
                // Navigating into such a folder causes a crash, which I am preventing with this try/catch block.
                // So, going into these folders will just show nothing instead of crashing.
            }
            std::error_code osError;
            for (auto i = fs::begin(fileIter); i != fs::end(fileIter); i = i.increment(osError))
            {
                if (osError) break;
                auto entry = *i;
                if (entry.is_directory() || 
                    (entry.is_regular_file() && _extensions.find(entry.path().extension().string()) != _extensions.end()))
                {
                    std::string entry_str = 
                        entry.is_directory() ? entry.path().stem().string()
                                             : entry.path().filename().string();
                    
                    bool selected = (strcmp(entry_str.c_str(), _fileNameBuffer) == 0);
                    if (ImGui::Selectable(entry_str.c_str(), selected))
                    {
                        if (entry.is_directory())
                        {
                            _currentDir = entry.path();
                            memset(_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
                        }
                        else
                        {
                            strcpy(_fileNameBuffer, entry_str.c_str());
                        }
                    }
                }
            }
            ImGui::EndListBox();
        }

        ImGui::InputText("File name", _fileNameBuffer, TEXT_FIELD_MAX);
        
        if (ImGui::Button("SELECT") && (strlen(_fileNameBuffer) > 0 || _extensions.empty()))
        {
            _callback(_currentDir.append(_fileNameBuffer));
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }
    return open;
}

static const int N_QUIT_MESSAGES = 10;
static const char *QUIT_MESSAGES[N_QUIT_MESSAGES] = {
    "Only winners take stretch breaks.", 
    "Did I leave the editor on Nightmare difficulty?", 
    "Remember to eat some clowns.", 
    "Admiring the *cough* robust C++ architecture?", 
    "Your SOUL is what needs saving, not this map file!",
    "Click 'Nah' to meet hot singles in your area!",
    "Whelp...I'm going to Grillby's...",
    "100% of people who go outside die!",
    "Да, я тоже не понимаю это приложение.",
    "They told me the fancy new UI would make you stay...",
};

CloseDialog::CloseDialog()
{
    _messageIdx = GetRandomValue(0, N_QUIT_MESSAGES - 1);
}

bool CloseDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup("REALLY QUIT?");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("REALLY QUIT?", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(QUIT_MESSAGES[_messageIdx]);

        if (ImGui::Button("Quit"))
        {
            App::Get()->Quit();
            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Nah"))
        {
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }
    return open;
}

AssetPathDialog::AssetPathDialog(App::Settings &settings)
    : _settings(settings),
    _fileDialog(nullptr)
{
    strcpy(_texDirBuffer, App::Get()->GetTexturesDir().c_str());
    strcpy(_shapeDirBuffer, App::Get()->GetShapesDir().c_str());
    strcpy(_defaultTexBuffer, App::Get()->GetDefaultTexturePath().c_str());
    strcpy(_defaultShapeBuffer, App::Get()->GetDefaultShapePath().c_str());
}

#define ASSET_PATH_FILE_DIALOG_CALLBACK(buffer)                         \
    [&](fs::path path)                                                  \
    {                                                                           \
        fs::path relativePath = fs::relative(path, fs::current_path());         \
        strcpy(buffer, relativePath.generic_string().c_str());           \
    }                                                                           \

bool AssetPathDialog::Draw()
{
    if (_fileDialog)
    {
        bool open = _fileDialog->Draw();
        if (!open) _fileDialog.reset(nullptr);
        return true;
    }

    bool open = true;
    ImGui::OpenPopup("CONFIGURE ASSET PATHS");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("CONFIGURE ASSET PATHS", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::Button("Browse##texdir"))
        {
            _fileDialog.reset(new FileDialog("Select textures directory", {}, 
                ASSET_PATH_FILE_DIALOG_CALLBACK(_texDirBuffer)
            ));
        }
        ImGui::SameLine();
        ImGui::InputText("Textures Directory", _texDirBuffer, TEXT_FIELD_MAX);

        if (ImGui::Button("Browse##deftex"))
        {
            _fileDialog.reset(new FileDialog("Select default texture", {".png"}, 
                ASSET_PATH_FILE_DIALOG_CALLBACK(_defaultTexBuffer)
            ));
        }
        ImGui::SameLine();
        ImGui::InputText("Default Texture", _defaultTexBuffer, TEXT_FIELD_MAX);

        if (ImGui::Button("Browse##shapedir"))
        {
            _fileDialog.reset(new FileDialog("Select shapes directory", {}, 
                ASSET_PATH_FILE_DIALOG_CALLBACK(_shapeDirBuffer)
            ));
        }
        ImGui::SameLine();
        ImGui::InputText("Shapes Directory", _shapeDirBuffer, TEXT_FIELD_MAX);

        if (ImGui::Button("Browse##defshape"))
        {
            _fileDialog.reset(new FileDialog("Select default shape", {".obj"}, 
                ASSET_PATH_FILE_DIALOG_CALLBACK(_defaultShapeBuffer)
            ));
        }
        ImGui::SameLine();
        ImGui::InputText("Default Shape", _defaultShapeBuffer, TEXT_FIELD_MAX);

        if (ImGui::Button("Confirm"))
        {
            // Validate all of the entered paths
            bool bad = false;
            fs::directory_entry texEntry { _texDirBuffer };
            if (!texEntry.exists() || !texEntry.is_directory())
            {
                strcpy(_texDirBuffer, "Invalid!");
                bad = true;
            }
            fs::directory_entry defTexEntry { _defaultTexBuffer };
            if (!defTexEntry.exists() || !defTexEntry.is_regular_file())
            {
                strcpy(_defaultTexBuffer, "Invalid!");
                bad = true;
            }
            fs::directory_entry shapeEntry { _shapeDirBuffer };
            if (!shapeEntry.exists() || !shapeEntry.is_directory())
            {
                strcpy(_shapeDirBuffer, "Invalid!");
                bad = true;
            }
            fs::directory_entry defShapeEntry { _defaultShapeBuffer };
            if (!defShapeEntry.exists() || !defShapeEntry.is_regular_file())
            {
                strcpy(_defaultShapeBuffer, "Invalid!");
                bad = true;
            }
            if (!bad) 
            {
                _settings.texturesDir = texEntry.path().string();
                _settings.defaultTexturePath = defTexEntry.path().string();
                _settings.shapesDir = shapeEntry.path().string();
                _settings.defaultShapePath = defShapeEntry.path().string();
                App::Get()->SaveSettings();
                ImGui::EndPopup();
                return false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }

    return open;

    // const Rectangle DRECT = DialogRec(632.0f, 320.0f);
    
    // bool clicked = GuiWindowBox(DRECT, "Asset Paths");

    // auto textBoxes = ArrangeVertical (
    //     Rectangle { DRECT.x + 8.0f, DRECT.y + 32.0f, DRECT.width - 16.0f, DRECT.height - 80.0f }, 
    //     Rectangle { .width = DRECT.width - 16.0f, .height = 24.0f },
    //     8
    // );

    // // Textures directory
    // GuiLabel(textBoxes[0], "Textures Directory");
    // if (GuiTextBox(textBoxes[1], _texPathBuffer, TEXT_FIELD_MAX, _texPathEdit))
    //     _texPathEdit = !_texPathEdit;
    
    // // Default texture
    // GuiLabel(textBoxes[2], "Default Texture Path");
    // if (GuiTextBox(textBoxes[3], _defaultTexBuffer, TEXT_FIELD_MAX, _defaultTexEdit))
    //     _defaultTexEdit = !_defaultTexEdit;

    // // Shape directory
    // GuiLabel(textBoxes[4], "Shapes Directory");
    // if (GuiTextBox(textBoxes[5], _shapePathBuffer, TEXT_FIELD_MAX, _shapePathEdit))
    //     _shapePathEdit = !_shapePathEdit;

    // // Default shape
    // GuiLabel(textBoxes[6], "Default Shape Path");
    // if (GuiTextBox(textBoxes[7], _defaultShapeBuffer, TEXT_FIELD_MAX, _defaultShapeEdit))
    //     _defaultShapeEdit = !_defaultShapeEdit;

    // // Submission buttons
    // const Rectangle BUTTON_REGION = Rectangle {
    //     .x = DRECT.x + 8.0f,
    //     .y = DRECT.y + DRECT.height - 48.0f,
    //     .width = DRECT.width - (BUTTON_REGION.x - DRECT.x) - 8.0f,
    //     .height = 48.0f
    // };
    // std::vector<Rectangle> buttonRecs = ArrangeHorzCentered(BUTTON_REGION, { 
    //     Rectangle{ .width = 96.0f, .height = 32.0f }, 
    //     Rectangle{ .width = 96.0f, .height = 32.0f } });

    // if (GuiButton(buttonRecs[0], "Confirm"))
    // {
    //     // Validate all of the entered paths
    //     bool bad = false;
    //     fs::directory_entry texEntry { _texPathBuffer };
    //     if (!texEntry.exists() || !texEntry.is_directory())
    //     {
    //         strcpy(_texPathBuffer, "Invalid!");
    //         bad = true;
    //     }
    //     fs::directory_entry defTexEntry { _defaultTexBuffer };
    //     if (!defTexEntry.exists() || !defTexEntry.is_regular_file())
    //     {
    //         strcpy(_defaultTexBuffer, "Invalid!");
    //         bad = true;
    //     }
    //     fs::directory_entry shapeEntry { _shapePathBuffer };
    //     if (!shapeEntry.exists() || !shapeEntry.is_directory())
    //     {
    //         strcpy(_shapePathBuffer, "Invalid!");
    //         bad = true;
    //     }
    //     fs::directory_entry defShapeEntry { _defaultShapeBuffer };
    //     if (!defShapeEntry.exists() || !defShapeEntry.is_regular_file())
    //     {
    //         strcpy(_defaultShapeBuffer, "Invalid!");
    //         bad = true;
    //     }
    //     if (!bad) 
    //     {
    //         _settings.texturesDir = texEntry.path().string();
    //         _settings.defaultTexturePath = defTexEntry.path().string();
    //         _settings.shapesDir = shapeEntry.path().string();
    //         _settings.defaultShapePath = defShapeEntry.path().string();
    //         App::Get()->SaveSettings();
    //         return false;
    //     }
    // }
    // else if (GuiButton(buttonRecs[1], "Cancel"))
    // {
    //     return false;
    // }

    // return !clicked;
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
        "Hold Middle click or LEFT ALT+LEFT CLICK - Look around",
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

    auto layoutRects = ArrangeVertical(Rectangle { DRECT.x + 8.0f, DRECT.y + 48.0f, DRECT.width - 16.0f, DRECT.height - 104.0f }, { 
        Rectangle { 0.0f, 0.0f, DRECT.width - 16.0f, 24.0f }, // File path box
        Rectangle { 0.0f, -6.0f, 128.0f, 32.0f }, // Browse button
        Rectangle { 0.0f, 0.0f, 32.0f, 32.0f }, // Separate nodes checkbox
        Rectangle { 0.0f, 0.0f, 32.0f, 32.0f }, // Cull faces checkbox
    });

    strcpy(_filePathBuffer, _settings.exportFilePath.c_str());
    if (GuiTextBox(layoutRects[0], _filePathBuffer, TEXT_FIELD_MAX, _filePathEdit))
    {
        _filePathEdit = !_filePathEdit;
    }   
    _settings.exportFilePath = _filePathBuffer;
    GuiLabel(Rectangle { .x = layoutRects[0].x, .y = layoutRects[0].y - 12.0f }, "File path");

    if (GuiButton(layoutRects[1], "Browse"))
    {
        _dialog.reset(new FileDialog(std::string("Save .GLTF file"), {std::string(".gltf")}, [&](fs::path path){
            _settings.exportFilePath = fs::relative(path).string();
        }));
    }

    _settings.exportSeparateGeometry = GuiCheckBox(layoutRects[2], "Separate nodes for each texture", _settings.exportSeparateGeometry);
    _settings.cullFaces = GuiCheckBox(layoutRects[3], "Cull redundant faces between tiles", _settings.cullFaces);

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
