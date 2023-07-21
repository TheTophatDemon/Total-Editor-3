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
}

SettingsDialog::SettingsDialog(App::Settings &settings)
    : _settings(settings),
      _undoMax(settings.undoMax),
      _sensitivity(settings.mouseSensitivity)
{
}

bool SettingsDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup("SETTINGS");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("SETTINGS", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputInt("Maximum undo count", &_undoMax, 1, 10);
        if (_undoMax < 0) _undoMax = 0;
        
        ImGui::SliderFloat("Mouse sensitivity", &_sensitivity, 0.05f, 10.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat);

        if (ImGui::Button("Confirm"))
        {
            _settings.undoMax = _undoMax;
            _settings.mouseSensitivity = _sensitivity;
            App::Get()->SaveSettings();
            ImGui::EndPopup();
            return false;
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
}

bool AboutDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup("ABOUT");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("ABOUT", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "Total Editor 3.1.0");
        ImGui::TextUnformatted("Written by The Tophat Demon\nWith help from Raylib, Nlohmann JSON, CPPCodec.\nHumble Fonts Gold II font made by Eevie Somepx");
        ImGui::TextColored(ImColor(0.0f, 0.5f, 0.5f), "Source code: \nhttps://github.com/TheTophatDemon/Total-Editor-3");

        ImGui::EndPopup();
        return true;
    }
    return open;
}

bool ShortcutsDialog::Draw()
{
    const char* SHORTCUT_KEYS[] = {
        "W/A/S/D",
        "Hold Middle click or LEFT ALT+LEFT CLICK",
        "Scroll wheel",
        "Left click",
        "Right click",
        "TAB",
        "LEFT SHIFT+TAB",
        "T (Tile mode)",
        "G (Tile mode)",
        "HOLD LEFT SHIFT",
        "Q",
        "E",
        "R",
        "F",
        "V",
        "H",
        "H (when layers are isolated)",
        "Hold H while using scrollwheel",
        "LEFT SHIFT+B",
        "ESCAPE/BACKSPACE",
        "LEFT CTRL+TAB",
        "LEFT CTRL+E",
        "T/G (Entity mode)",
        "LEFT CTRL+S",
        "LEFT CTRL+Z",
        "LEFT CTRL+Y"
    };

    const char* SHORTCUT_INFO[] = {
        "Move camera",
        "Look around",
        "Move grid up/down",
        "Place tile/entity/brush",
        "Remove tile/entity (Does not work in brush mode)",
        "Switch between texture picker and map editor.",
        "Switch between shape picker and map editor.",
        "Select texture of tile under cursor",
        "Select shape of tile under cursor",
        "Expand cursor to place tiles in bulk.",
        "Turn cursor counterclockwise",
        "Turn cursor clockwise",
        "Reset cursor orientation",
        "Turn cursor upwards",
        "Turn cursor downwards",
        "Isolate the layer of tiles the grid is on.",
        "Unhide hidden layers.",
        "Select multiple layers to isolate.",
        "Capture tiles under cursor as a brush.",
        "Return cursor to tile mode.",
        "Switch between entity editor and map editor.",
        "Put cursor into entity mode.",
        "Copy entity from under cursor.",
        "Save map.",
        "Undo",
        "Redo"
    };

    const int SHORTCUT_COUNT = 26;

    bool open = true;
    ImGui::OpenPopup("SHORTCUTS");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(608.0f, 468.0f));
    if (ImGui::BeginPopupModal("SHORTCUTS", &open, ImGuiWindowFlags_AlwaysVerticalScrollbar))
    {
        for (int i = 0; i < SHORTCUT_COUNT; ++i)
        {
            ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), SHORTCUT_KEYS[i]);
            ImGui::SameLine();
            ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "-");
            ImGui::SameLine();
            ImGui::TextUnformatted(SHORTCUT_INFO[i]);
        }

        ImGui::EndPopup();
        return true;
    }
    return open;
}

bool InstructionsDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup("INSTRUCTIONS");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400.0f, 160.0f));
    if (ImGui::BeginPopupModal("INSTRUCTIONS", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextWrapped("Please refer to the instructions.html file included with the application.");

        ImGui::EndPopup();
        return true;
    }
    return open;
}

ExportDialog::ExportDialog(App::Settings &settings)
    : _settings(settings)
{
    strcpy(_filePathBuffer, _settings.exportFilePath.c_str());
}

bool ExportDialog::Draw()
{
    if (_dialog)
    {
        if (!_dialog->Draw())
            _dialog.reset(nullptr);
        return true;
    }

    bool open = true;
    ImGui::OpenPopup("EXPORT .GLTF/.GLB SCENE");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("EXPORT .GLTF/.GLB SCENE", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(".gltf and .glb are both supported!");
        ImGui::InputText("File path", _filePathBuffer, TEXT_FIELD_MAX);
        ImGui::SameLine();
        if (ImGui::Button("Browse##gltfexport"))
        {
            _dialog.reset(new FileDialog(std::string("Save .GLTF or .GLB file"), {std::string(".gltf"), std::string(".glb")}, [&](fs::path path){
                _settings.exportFilePath = fs::relative(path).string();
                strcpy(_filePathBuffer, _settings.exportFilePath.c_str());
            }));
        }
        _settings.exportFilePath = _filePathBuffer;

        ImGui::Checkbox("Seperate nodes for each texture", &_settings.exportSeparateGeometry);
        ImGui::Checkbox("Cull redundant faces between tiles", &_settings.cullFaces);

        if (ImGui::Button("Export##exportgltf"))
        {
            App::Get()->TryExportMap(fs::path(_settings.exportFilePath), _settings.exportSeparateGeometry);
            App::Get()->SaveSettings();
            ImGui::EndPopup();
            return false;
        }

        ImGui::EndPopup();
        return true;
    }

    return open;
}
