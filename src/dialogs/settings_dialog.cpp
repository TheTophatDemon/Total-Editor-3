#include "dialogs.hpp"

#include "raylib.h"
#include "imgui/imgui.h"

#include "../app.hpp"
#include "../map_man/map_man.hpp"
#include "../defer.hpp"

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
        DEFER(ImGui::EndPopup());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{8.0f, 8.0f});
        DEFER(ImGui::PopStyleVar());

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
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            return false;
        }
        
        return true;
    }
    return open;
}