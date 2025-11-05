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

bool AboutDialog::Draw()
{
    bool open = true;
    ImGui::OpenPopup("ABOUT");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("ABOUT", &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "Total Editor 3.3.0");
        ImGui::TextUnformatted("Written by The Tophat Demon\nWith help from Raylib, Nlohmann JSON, CPPCodec, ImGUI.\nHumble Fonts Gold II font made by Eevie Somepx");
        ImGui::TextColored(ImColor(0.0f, 0.5f, 0.5f), "Source code: \nhttps://github.com/TheTophatDemon/Total-Editor-3");

        ImGui::EndPopup();
        return true;
    }
    return open;
}