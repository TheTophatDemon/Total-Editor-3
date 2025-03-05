#include "dialogs.hpp"

#include "imgui/imgui.h"

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