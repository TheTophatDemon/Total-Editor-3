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

#include "menu_bar.hpp"

#include "imgui/imgui.h"

#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;

#include "map_man/map_man.hpp"

MenuBar::MenuBar(App::Settings& settings, MapMan& mapMan): 
    _settings(settings), 
    _mapMan(mapMan),
	_activeDialog(nullptr),
	_messageTimer(0.0f),
	_messagePriority(0)
{
}

void MenuBar::DisplayStatusMessage(std::string message, float durationSeconds, int priority)
{
    if (priority >= _messagePriority)
    {
        _statusMessage = message;
        _messageTimer = durationSeconds;
        _messagePriority = priority;
    }
}

void MenuBar::OpenOpenMapDialog()
{
    auto makeFileDialog = [this] { 
        return new FileDialog(
            "Open Map (*.te3)",
            std::initializer_list<std::string>{ ".te3" }, 
            [this](fs::path path)
            { 
                App::Get()->TryOpenMap(path);
                if (_mapMan.WillConvert())
                {
                    _activeDialog.reset(new ConfirmationDialog(
                        "CONVERSION WARNING",
                        "This map will be converted to the new format upon saving.\n"
                            "This change cannot be undone.",
                        "", "Ok",
                        nullptr
                    ));
                }
            }, 
            false
        ); 
    };
    
    if (_mapMan.HasUnsavedChanges())
    {
        _activeDialog.reset(
            new ConfirmationDialog(
                "DISCARD CHANGES?", 
                "Shall you forsake that which you have created?", 
                "Discard", "Cancel", 
                [this, makeFileDialog](bool proceed) { 
                    if (!proceed) return;
                    _activeDialog.reset(makeFileDialog());
                }
            )
        );
    }
    else
    {
        _activeDialog.reset(makeFileDialog());
    }
}

void MenuBar::OpenSaveMapDialog()
{
    auto callback = [](fs::path path)
    { 
        App::Get()->TrySaveMap(path); 
    };
    _activeDialog.reset(new FileDialog("Save Map (*.te3)", { ".te3" }, callback, true)); 
}

void MenuBar::SaveMap()
{
    if (!App::Get()->GetLastSavedPath().empty())
    {
        App::Get()->TrySaveMap(App::Get()->GetLastSavedPath());
    }
    else
    {
        OpenSaveMapDialog();
    }
}

void MenuBar::Update()
{
    if (_messageTimer > 0.0f)
    {
        _messageTimer -= GetFrameTime();
        if (_messageTimer <= 0.0f)
        {
            _messageTimer = 0.0f;
            _messagePriority = 0;
        }
    }
    
    // Show exit confirmation dialog
    if (WindowShouldClose()) 
    {
        static const char *QUIT_MESSAGES[] = {
            "Only winners take stretch breaks.", 
            "Did I leave the editor on Nightmare difficulty?", 
            "Remember to eat some clowns.", 
            "Admiring the *cough* robust C++ architecture?", 
            "Your SOUL is what needs saving, not this map file!",
            "Click 'Nah' to meet hot singles in your area!",
            "Whelp...I'm going to Grillby's...",
            "100% of people who go outside die!",
            "Да, я тоже не понимаю это приложение.",
            "They told me the fancy new UI would make you stay...",
            "Support me today or I'll edit your face!",
            "Dame da ne...dame yo...dame na no yo...",
            "So you've got better things to do, huh?"
        };
        int messageIdx = GetRandomValue(0, (sizeof(QUIT_MESSAGES) / sizeof(char *)) - 1);

        _activeDialog.reset(new ConfirmationDialog(
            "REALLY QUIT?",
            QUIT_MESSAGES[messageIdx],
            "Quit", "Nah",
            [](bool proceed) 
            {
                if (proceed) App::Get()->Quit();
            }
        ));
    }
}

void MenuBar::Draw()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("MAP"))
        {
            if (ImGui::MenuItem("NEW")) _activeDialog.reset(new NewMapDialog());
            if (ImGui::MenuItem("OPEN")) OpenOpenMapDialog();
            if (ImGui::MenuItem("SAVE")) SaveMap();
            if (ImGui::MenuItem("SAVE AS")) OpenSaveMapDialog();
            if (ImGui::MenuItem("EXPORT")) _activeDialog.reset(new ExportDialog(_settings));
            if (ImGui::MenuItem("EXPAND GRID")) _activeDialog.reset(new ExpandMapDialog());
            if (ImGui::MenuItem("SHRINK GRID")) 
            {
                _activeDialog.reset(new ConfirmationDialog(
                    "SHRINK GRID",
                    "Shrink the grid to fit around the existing tiles?",
                    "Yes", "No",
                    [](bool proceed)
                    { 
                        if (proceed) App::Get()->ShrinkMap(); 
                    }
                ));
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("VIEW"))
        {
            if (ImGui::MenuItem("MAP EDITOR"))     App::Get()->ChangeEditorMode(App::Mode::PLACE_TILE);
            if (ImGui::MenuItem("TEXTURE PICKER")) App::Get()->ChangeEditorMode(App::Mode::PICK_TEXTURE);
            if (ImGui::MenuItem("SHAPE PICKER"))   App::Get()->ChangeEditorMode(App::Mode::PICK_SHAPE);
            if (ImGui::MenuItem("ENTITY EDITOR"))  App::Get()->ChangeEditorMode(App::Mode::EDIT_ENT);
            if (ImGui::MenuItem("RESET CAMERA"))   App::Get()->ResetEditorCamera();
            if (ImGui::MenuItem("TOGGLE PREVIEW")) App::Get()->TogglePreviewing();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("CONFIG"))
        {
            if (ImGui::MenuItem("ASSET PATHS")) _activeDialog.reset(new AssetPathDialog(_settings));
            if (ImGui::MenuItem("SETTINGS"))    _activeDialog.reset(new SettingsDialog(_settings));
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("INFO"))
        {
            if (ImGui::MenuItem("ABOUT"))          _activeDialog.reset(new AboutDialog());
            if (ImGui::MenuItem("KEYS/SHORTCUTS")) _activeDialog.reset(new ShortcutsDialog());
            if (ImGui::MenuItem("INSTRUCTIONS"))   _activeDialog.reset(new InstructionsDialog());
            ImGui::EndMenu();
        }
        
        ImGui::SameLine();
        ImGui::TextUnformatted(" | ");
        if (_messageTimer > 0.0f) 
        {
            ImGui::SameLine();
            ImGui::TextUnformatted(_statusMessage.c_str());
        }
        
        ImGui::EndMainMenuBar();
    }
    
    // Draw modal dialogs
    if (Dialog* dialog = _activeDialog.get(); dialog)
    {
        // Reset to no active dialog unless it was changed by a callback.
        if (!dialog->Draw() && _activeDialog.get() == dialog)
        {
            _activeDialog.reset(nullptr);
        }
    }
}
