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
#include "imgui/imgui.h"
#include "imgui/rlImGui.h"

#include <vector>
#include <initializer_list>
#include <map>
#include <iostream>
#include <set>
#include <string>
#include <regex>

#include "../assets.hpp"
#include "../math_stuff.hpp"
#include "../app.hpp"
#include "../map_man.hpp"
#include "../text_util.hpp"
#include "../draw_extras.h"
#include "../defer.hpp"

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

FileDialog::FileDialog(std::string title, std::initializer_list<std::string> extensions, std::function<void(std::filesystem::path)> callback, bool writeMode) 
    : _title(title),
      _extensions(extensions),
      _callback(callback),
      _currentDir(fs::current_path()),
      _overwritePromptOpen(false),
      _writeMode(writeMode)
{
    memset(&_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
}

bool FileDialog::Draw()
{
    bool open = true;
    if (!_overwritePromptOpen) ImGui::OpenPopup(_title.c_str());
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(_title.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Parent directory button & other controls
        
        if (ImGui::Button("Parent directory") && _currentDir.has_parent_path())
        {
            _currentDir = _currentDir.parent_path();
        }

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

            // Store found files and directories in these sets so that they get automatically sorted (and we can put directories in front of files)
            std::set<fs::path> foundDirectories;
            std::set<fs::path> foundFiles;

            // Get files/directories from the file system
            std::error_code osError;
            for (auto i = fs::begin(fileIter); i != fs::end(fileIter); i = i.increment(osError))
            {
                if (osError) break;
                auto entry = *i;
                if (entry.is_directory())
                {
                    foundDirectories.insert(entry.path());
                }
                if (entry.is_regular_file() && _extensions.find(entry.path().extension().string()) != _extensions.end())
                {
                    foundFiles.insert(entry.path());
                }
            }

            // Display the directories
            for (fs::path entry : foundDirectories)
            {
                std::string entry_str = std::string("[") + entry.stem().string() + "]";
                // Directories are yellow
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                
                if (ImGui::Selectable(entry_str.c_str()))
                {
                    _currentDir = entry;
                    memset(_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
                }
                ImGui::PopStyleColor();
            }

            // Display the files
            for (fs::path entry : foundFiles)
            {
                std::string entry_str = entry.filename().string();
                // Files are white
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                if (ImGui::Selectable(entry_str.c_str()))
                {
                    strcpy(_fileNameBuffer, entry_str.c_str());
                }
                ImGui::PopStyleColor();
            }

            ImGui::EndListBox();
        }

        ImGui::InputText("File name", _fileNameBuffer, TEXT_FIELD_MAX);
        
        if (ImGui::Button("SELECT") && (strlen(_fileNameBuffer) > 0 || _extensions.empty()))
        {
            fs::path newPath = _currentDir / _fileNameBuffer;
            if (_writeMode && fs::exists(newPath))
            {
                _overwritePromptOpen = true;
                _overwriteDir = newPath;
            }
            else
            {
                _callback(newPath);

                ImGui::EndPopup();
                return false;
            }
        }

        ImGui::EndPopup();
    }

    if (_overwritePromptOpen) ImGui::OpenPopup("CONFIRM OVERWRITE?");
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("CONFIRM OVERWRITE?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(_overwriteDir.string().c_str());
        ImGui::TextUnformatted("Do you DARE overwrite this file?");
        
        if (ImGui::Button("YES"))
        {
            _callback(_overwriteDir);

            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("NO"))
        {
            ImGui::CloseCurrentPopup();
            _overwritePromptOpen = false;
        }

        ImGui::EndPopup();
    }

    return open;
}

static const int N_QUIT_MESSAGES = 11;
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
    "Donate now or I'll edit your face!",
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

SettingsDialog::SettingsDialog(App::Settings &settings)
    : _settingsOriginal(settings),
      _settingsCopy(settings)
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
        int undoMax = (int)_settingsCopy.undoMax;
        ImGui::InputInt("Maximum undo count", &undoMax, 1, 10);
        if (undoMax < 0) undoMax = 0;
        _settingsCopy.undoMax = undoMax;
        
        ImGui::SliderFloat("Mouse sensitivity", &_settingsCopy.mouseSensitivity, 0.05f, 10.0f, "%.1f", ImGuiSliderFlags_NoRoundToFormat);

        float bgColorf[3] = { 
            (float)std::get<0>(_settingsCopy.backgroundColor) / 255.0f, 
            (float)std::get<1>(_settingsCopy.backgroundColor) / 255.0f, 
            (float)std::get<2>(_settingsCopy.backgroundColor) / 255.0f 
        };
        ImGui::SetNextItemWidth(256.0f);
        ImGui::ColorPicker3("Background color", bgColorf, ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB);
        _settingsCopy.backgroundColor = std::make_tuple(
            (uint8_t)(bgColorf[0] * 255.0f), 
            (uint8_t)(bgColorf[1] * 255.0f), 
            (uint8_t)(bgColorf[2] * 255.0f));

        if (ImGui::Button("Confirm"))
        {
            _settingsOriginal = _settingsCopy;
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
        ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "Total Editor 3.2.0");
        ImGui::TextUnformatted("Written by The Tophat Demon\nWith help from Raylib, Nlohmann JSON, CPPCodec, ImGUI.\nHumble Fonts Gold II font made by Eevie Somepx");
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
    ImGui::SetNextWindowSizeConstraints(ImVec2(640.0f, 468.0f), ImVec2(1280.0f, 720.0f));
    if (ImGui::BeginPopupModal("SHORTCUTS", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
    {
        for (int i = 0; i < SHORTCUT_COUNT; ++i)
        {
            ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "%s", SHORTCUT_KEYS[i]);
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
            }, true));
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