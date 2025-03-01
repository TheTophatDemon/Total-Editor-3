#include "dialogs.hpp"

#include "imgui/imgui.h"

ConfirmationDialog::ConfirmationDialogConfirmationDialog(const std::string& titleText, const std::string& bodyText, const std::string& confirmText, 
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
    ImGui::OpenPopup(_titleText);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(_titleText, NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(_bodyText);
        
        if (ImGui::Button(_confirmText))
        {
            _callback(true);

            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button(_cancelText))
        {
            _callback(false);

            ImGui::CloseCurrentPopup();
            return false;
        }

        ImGui::EndPopup();
    }

    return true;
}