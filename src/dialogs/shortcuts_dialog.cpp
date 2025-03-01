#include "dialogs.hpp"

#include "imgui/imgui.h"

bool ShortcutsDialog::Draw()
{
    static const char *SHORTCUT_STRINGS[] = {
        "W/A/S/D", "Move camera",
        "Hold Middle click or LEFT ALT+LEFT CLICK", "Look around",
        "Scroll wheel", "Move grid up/down",
        "Left click", "Place tile/entity/brush",
        "Right click", "Remove tile/entity (Does not work in brush mode)",
        "TAB", "Switch between texture picker and map editor.",
        "LEFT SHIFT+TAB", "Switch between shape picker and map editor.",
        "T (Tile mode)", "Select textures of tile under cursor",
        "Y (Tile mode)", "Select secondary texture of tile under cursor",
        "G (Tile mode)", "Select shape of tile under cursor",
        "HOLD LEFT SHIFT", "Expand cursor to place tiles in bulk.",
        "Q", "Turn cursor counterclockwise",
        "E", "Turn cursor clockwise",
        "R", "Reset cursor orientation",
        "F", "Turn cursor upwards",
        "V", "Turn cursor downwards",
        "H", "Isolate the layer of tiles the grid is on.",
        "H (when layers are isolated)", "Unhide hidden layers.",
        "Hold H while using scrollwheel", "Select multiple layers to isolate.",
        "LEFT SHIFT+B", "Capture tiles under cursor as a brush.",
        "ESCAPE/BACKSPACE", "Return cursor to tile mode.",
        "LEFT CTRL+TAB", "Switch between entity editor and map editor.",
        "LEFT CTRL+E", "Put cursor into entity mode.",
        "T/G (Entity mode)", "Copy entity from under cursor.",
        "LEFT CTRL+S", "Save map.",
        "LEFT CTRL+Z", "Undo",
        "LEFT CTRL+Y", "Redo"
    };

    static const int SHORTCUT_COUNT = sizeof(SHORTCUT_STRINGS) / sizeof(char *) / 2;

    bool open = true;
    ImGui::OpenPopup("SHORTCUTS");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(640.0f, 468.0f), ImVec2(1280.0f, 720.0f));
    if (ImGui::BeginPopupModal("SHORTCUTS", &open, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
    {
        for (int i = 0; i < SHORTCUT_COUNT; ++i)
        {
            ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "%s", SHORTCUT_STRINGS[i*2]);
            ImGui::SameLine();
            ImGui::TextColored(ImColor(1.0f, 1.0f, 0.0f), "-");
            ImGui::SameLine();
            ImGui::TextUnformatted(SHORTCUT_STRINGS[(i*2) + 1]);
        }

        ImGui::EndPopup();
        return true;
    }
    return open;
}