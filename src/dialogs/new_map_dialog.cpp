#include "dialogs.hpp"

#include "imgui/imgui.h"

#include "../app.hpp"
#include "../map_man/map_man.hpp"

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