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

ConfirmationDialog::ConfirmationDialog(const std::string& titleText, const std::string& bodyText, const std::string& confirmText, 
                                       const std::string& cancelText, std::function<void(bool)> callback)
{
    _titleText = titleText;
    _bodyText = bodyText;
    _confirmText = confirmText;
    _cancelText = cancelText;
    _callback = callback;
}

bool ConfirmationDialog::Draw()
{
    ImGui::OpenPopup(_titleText.c_str());
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(_titleText.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(_bodyText.c_str());
        
        if (!_confirmText.empty())
        {
            if (ImGui::Button(_confirmText.c_str()))
            {
                if (_callback) _callback(true);
                
                ImGui::EndPopup();
                return false;
            }
            ImGui::SameLine();
        }
        if (ImGui::Button(_cancelText.c_str()))
        {
            if (_callback) _callback(false);
            
            ImGui::EndPopup();
            return false;
        }
        
        ImGui::EndPopup();
    }
    
    return true;
}
