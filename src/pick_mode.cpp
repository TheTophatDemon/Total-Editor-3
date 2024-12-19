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

#include "pick_mode.hpp"

#include "raymath.h"
#include "imgui/rlImGui.h"

#include <cstring>
#include <stack>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <stdio.h>

#include "assets.hpp"
#include "text_util.hpp"

#define FRAME_SIZE 196
#define ICON_SIZE 64

PickMode::Frame::Frame()
    : Frame(fs::path(), fs::path())
{}

PickMode::Frame::Frame(const fs::path filePath, const fs::path rootDir)
{
    this->filePath = filePath;
    label = fs::relative(filePath, rootDir).string();
}

PickMode::PickMode(Mode mode)
    : _mode(mode)
{
    memset(_searchFilterBuffer, 0, sizeof(char) * SEARCH_BUFFER_SIZE);
    
    _iconCamera = Camera {
        .position = Vector3 { 4.0f, 4.0f, 4.0f },
        .target = Vector3Zero(),
        .up = Vector3 { 0.0f, -1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };
}

std::shared_ptr<Assets::TexHandle> PickMode::GetPickedTexture() const
{ 
    assert(_mode == Mode::TEXTURES);
    if (_selectedFrame.filePath.empty())
        return Assets::GetTexture(fs::path(App::Get()->GetDefaultTexturePath()));
    else
        return Assets::GetTexture(_selectedFrame.filePath);
}

std::shared_ptr<Assets::ModelHandle> PickMode::GetPickedShape() const
{
    assert(_mode == Mode::SHAPES);
    if (_selectedFrame.filePath.empty())
        return Assets::GetModel(fs::path(App::Get()->GetDefaultShapePath()));
    else
        return Assets::GetModel(_selectedFrame.filePath);
}

Texture2D PickMode::_GetTexture(const fs::path path)
{
    if (_loadedTextures.find(path) == _loadedTextures.end())
    {
        // Before loading the texture, resize it to fit into the frame
        Image image = LoadImage(path.string().c_str());
        ImageResize(&image, ICON_SIZE, ICON_SIZE);
        Texture2D texture = LoadTextureFromImage(image);
        UnloadImage(image);
        _loadedTextures[path] = texture;
    }
    return _loadedTextures[path];
}

Model PickMode::_GetModel(const fs::path path)
{
    if (_loadedModels.find(path) == _loadedModels.end())
    {
        Model model = LoadModel(path.string().c_str());
        _loadedModels[path] = model;
        return model;
    }
    return _loadedModels[path];
}

RenderTexture2D PickMode::_GetIcon(const fs::path path)
{
    if (_loadedIcons.find(path) == _loadedIcons.end())
    {
        RenderTexture2D icon = LoadRenderTexture(ICON_SIZE, ICON_SIZE);
        _loadedIcons[path] = icon;
        return icon;
    }
    return _loadedIcons[path];
}

void PickMode::_GetFrames()
{
    _frames.clear();
    
    for (const fs::path path : _foundFiles)
    {
        Frame frame(path, _rootDir);
        frame.texture = Texture {
            .id = 0,
            .width = ICON_SIZE,
            .height = ICON_SIZE,
            .mipmaps = 0,
            .format = 0,
        };

        //Filter out files that don't contain the search term
        std::string lowerCaseLabel = TextToLower(frame.label.c_str());
        if (strlen(_searchFilterBuffer) > 0 && 
            lowerCaseLabel.find(TextToLower(_searchFilterBuffer)) == std::string::npos)
        {
            continue;
        }

        _frames.push_back(frame);
    }
}

void PickMode::OnEnter()
{
    // Get the paths to all assets if this hasn't been done already.
    _rootDir = (_mode == Mode::SHAPES) ? fs::path(App::Get()->GetShapesDir()) : fs::path(App::Get()->GetTexturesDir());
    if (_foundFiles.empty())
    {
        if (!fs::is_directory(_rootDir)) 
        {
            std::cerr << "Asset directory in settings is not a directory!" << std::endl;
            return;
        }
        
        for (auto const& entry : fs::recursive_directory_iterator{_rootDir})
        {
            if (entry.is_directory() || !entry.is_regular_file()) continue;
            
            //Filter out files with the wrong extensions
            auto extensionStr = TextToLower(entry.path().extension().string().c_str());
            if (
                (_mode != Mode::TEXTURES || !TextIsEqual(extensionStr, ".png")) &&
                (_mode != Mode::SHAPES   || !TextIsEqual(extensionStr, ".obj"))
            ) {
                continue;
            }

            _foundFiles.insert(entry.path());
        }
    }

    _GetFrames();
}

void PickMode::OnExit()
{
    for (const auto& pair : _loadedModels)
    {
        UnloadModel(pair.second);
    }
    _loadedModels.clear();
    
    for (const auto& pair : _loadedTextures)
    {
        UnloadTexture(pair.second);
    }
    _loadedTextures.clear();

    for (const auto& pair : _loadedIcons)
    {
        UnloadRenderTexture(pair.second);
    }
    _loadedIcons.clear();
}

void PickMode::Update()
{
    if (_mode == Mode::SHAPES)
    {
        for (Frame& frame : _frames) 
        {
            //Update/redraw the shape preview icons so that they spin
            BeginTextureMode(_GetIcon(frame.filePath));
            ClearBackground(BLACK);
            BeginMode3D(_iconCamera);

            DrawModelWiresEx(_GetModel(frame.filePath), Vector3Zero(), Vector3{0.0f, 1.0f, 0.0f}, float(GetTime() * 180.0f), Vector3One(), GREEN);

            EndMode3D();
            EndTextureMode();
        }
    }
}

void PickMode::Draw()
{
    const float WINDOW_UPPER_MARGIN = 24.0f;
    bool open = true;
    ImGui::SetNextWindowSize(ImVec2((float)GetScreenWidth(), (float)GetScreenHeight() - WINDOW_UPPER_MARGIN));
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(ImVec2(center.x, center.y + WINDOW_UPPER_MARGIN), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("##Pick Mode View", &open, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove))
    {
        ImVec2 windowSize = ImGui::GetItemRectSize();
        
        if (ImGui::InputText("Search", _searchFilterBuffer, SEARCH_BUFFER_SIZE))
        {
            _GetFrames();
        }

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0, 8.0));
        ImGui::Separator();
        ImGui::PopStyleVar(1);
        
        const int NUM_COLS = Max((int)((windowSize.x) / (FRAME_SIZE * 1.5f)), 1);
        const int NUM_ROWS = Max((int)ceilf(_frames.size() / (float)NUM_COLS), 1);
        
        if (ImGui::BeginTable("##Frames", NUM_COLS, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_ScrollY))
        {
            for (int r = 0; r < NUM_ROWS; ++r)
            {
                ImGui::TableNextRow();
                for (int c = 0; c < NUM_COLS; ++c)
                {
                    ImGui::TableNextColumn();

                    int frameIndex = c + r * NUM_COLS;
                    if (frameIndex >= _frames.size()) break;

                    fs::path filePath = _frames[frameIndex].filePath;
                    
                    ImColor color = ImColor(1.0f, 1.0f, 1.0f);
                    if (_selectedFrame.filePath == _frames[frameIndex].filePath)
                    {
                        // Set color when selected to yellow
                        color = ImColor(1.0f, 1.0f, 0.0f);
                    }

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color));
                    if (rlImGuiImageButton(filePath.string().c_str(), &_frames[frameIndex].texture))
                    {
                        _selectedFrame = _frames[frameIndex];
                    }
                    ImGui::PopStyleColor(1);

                    if (ImGui::IsItemVisible() && !IsTextureValid(_frames[frameIndex].texture)) 
                    {
                        switch (_mode)
                        {
                        case Mode::TEXTURES: _frames[frameIndex].texture = _GetTexture(filePath); break;
                        case Mode::SHAPES: _frames[frameIndex].texture = _GetIcon(filePath).texture; break;
                        }
                    }

                    ImGui::TextColored(color, _frames[frameIndex].label.c_str());
                }
            }
            ImGui::EndTable();
        }

        ImGui::End();
    }
}
