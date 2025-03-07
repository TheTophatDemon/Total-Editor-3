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

#include "imgui/imgui.h"

#include "../app.hpp"
#include "../map_man/map_man.hpp"

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
        {
            _dialog.reset(nullptr);
        }
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
            _dialog.reset(new FileDialog(
                std::string("Save .GLTF or .GLB file"), 
                {std::string(".gltf"), std::string(".glb")}, 
                [&](fs::path path)
                {
                    _settings.exportFilePath = fs::relative(path).string();
                    strcpy(_filePathBuffer, _settings.exportFilePath.c_str());
                }, 
                true
            ));
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