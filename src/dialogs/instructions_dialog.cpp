#include "dialogs.hpp"

#include "imgui/imgui.h"

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