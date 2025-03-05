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

#include <string>
#include <regex>
#include <iostream>

AssetPathDialog::AssetPathDialog(App::Settings &settings)
    : _settings(settings),
    _fileDialog(nullptr)
{
    strcpy(_texDirBuffer, App::Get()->GetTexturesDir().c_str());
    strcpy(_shapeDirBuffer, App::Get()->GetShapesDir().c_str());
    strcpy(_defaultTexBuffer, App::Get()->GetDefaultTexturePath().c_str());
    strcpy(_defaultShapeBuffer, App::Get()->GetDefaultShapePath().c_str());
    strcpy(_hiddenAssetRegexBuffer, _settings.assetHideRegex.c_str());
    _hiddenAssetRegexValid = true;
}

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
        #define FILE_DIALOG_CALLBACK(buffer)                                    \
            [&](fs::path path)                                                  \
            {                                                                   \
                fs::path relativePath = fs::relative(path, fs::current_path()); \
                strcpy(buffer, relativePath.generic_string().c_str());          \
            }
        
        if (ImGui::Button("Browse##texdir"))
        {
            _fileDialog.reset(new FileDialog("Select textures directory", {}, FILE_DIALOG_CALLBACK(_texDirBuffer), false));
        }
        ImGui::SameLine();
        ImGui::InputText("Textures Directory", _texDirBuffer, TEXT_FIELD_MAX);

        if (ImGui::Button("Browse##deftex"))
        {
            _fileDialog.reset(new FileDialog("Select default texture", {".png"}, FILE_DIALOG_CALLBACK(_defaultTexBuffer), false));
        }
        ImGui::SameLine();
        ImGui::InputText("Default Texture", _defaultTexBuffer, TEXT_FIELD_MAX);

        if (ImGui::Button("Browse##shapedir"))
        {
            _fileDialog.reset(new FileDialog("Select shapes directory", {}, FILE_DIALOG_CALLBACK(_shapeDirBuffer), false));
        }
        ImGui::SameLine();
        ImGui::InputText("Shapes Directory", _shapeDirBuffer, TEXT_FIELD_MAX);

        if (ImGui::Button("Browse##defshape"))
        {
            _fileDialog.reset(new FileDialog("Select default shape", {".obj"}, FILE_DIALOG_CALLBACK(_defaultShapeBuffer), false));
        }
        ImGui::SameLine();
        ImGui::InputText("Default Shape", _defaultShapeBuffer, TEXT_FIELD_MAX);

        #undef FILE_DIALOG_CALLBACK

        ImGui::PushFont(Assets::GetCodeFont());
        ImGui::InputText("Hidden Asset Regex", _hiddenAssetRegexBuffer, TEXT_FIELD_MAX);
        ImGui::PopFont();
        ImVec4 errorColor = {1.0f, 0.0f, 0.0f, 1.0f};
        ImGui::TextColored(errorColor, _hiddenAssetRegexValid ? "" : "Invalid");

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

            // Validate the regular expression
            try 
            {
                std::regex hiddenAssetRegex(_hiddenAssetRegexBuffer);
                _hiddenAssetRegexValid = true;
            }
            catch (const std::regex_error &err)
            {
                std::cerr << "Error validation hidden asset regex: " << err.what() << std::endl;
                _hiddenAssetRegexValid = false;
                bad = true;
            }

            if (!bad) 
            {
                _settings.texturesDir = texEntry.path().string();
                _settings.defaultTexturePath = defTexEntry.path().string();
                _settings.shapesDir = shapeEntry.path().string();
                _settings.defaultShapePath = defShapeEntry.path().string();
                _settings.assetHideRegex = std::string(_hiddenAssetRegexBuffer);
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