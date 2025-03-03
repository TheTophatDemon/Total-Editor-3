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
#include "../map_man/map_man.hpp"
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
    "Support us today or we'll edit your face!",
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