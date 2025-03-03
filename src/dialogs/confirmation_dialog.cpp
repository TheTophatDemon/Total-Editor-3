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
        
        if (ImGui::Button(_confirmText.c_str()))
        {
            _callback(true);
            
            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button(_cancelText.c_str()))
        {
            _callback(false);
            
            ImGui::EndPopup();
            return false;
        }
        
        ImGui::EndPopup();
    }
    
    return true;
}
