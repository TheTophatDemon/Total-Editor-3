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

#include "extras/raygui.h"
#include "raymath.h"

#include <cstring>
#include <stack>
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <stdio.h>

#include "assets.hpp"
#include "text_util.hpp"

#define UPPER_MARGIN 48
#define FRAME_SIZE 64
#define FRAME_MARGIN 16
#define FRAME_SPACING (FRAME_SIZE + FRAME_MARGIN * 2)

PickMode::Frame::Frame()
    : Frame(fs::path(), fs::path())
{}

PickMode::Frame::Frame(const fs::path filePath, const fs::path rootDir)
{
    this->filePath = filePath;
    label = fs::relative(filePath, rootDir).string();
}

PickMode::PickMode(Mode mode)
    : _mode(mode),
      _scroll(Vector2Zero()),
      _searchFilterFocused(false),
      _view(mode == Mode::TEXTURES ? View::GRID : View::LIST),
      _longestLabelLength(0)
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
        Texture2D texture = LoadTexture(path.string().c_str());
        _loadedTextures[path] = texture;
        return texture;
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
        RenderTexture2D icon = LoadRenderTexture(FRAME_SIZE, FRAME_SIZE);
        _loadedIcons[path] = icon;
        return icon;
    }
    return _loadedIcons[path];
}

void PickMode::_GetFrames(fs::path rootDir)
{
    _frames.clear();
    
    for (const fs::path path : _foundFiles)
    {
        Frame frame(path, rootDir);

        //Filter out files that don't contain the search term
        std::string lowerCaseLabel = TextToLower(frame.label.c_str());
        if (strlen(_searchFilterBuffer) > 0 && 
            lowerCaseLabel.find(TextToLower(_searchFilterBuffer)) == std::string::npos)
        {
            continue;
        }

        //Update longest label length
        _longestLabelLength = Max(_longestLabelLength, frame.label.length());

        _frames.push_back(std::move(frame));
    }
}

void PickMode::OnEnter()
{
    // Get the paths to all assets if this hasn't been done already.
    auto rootDir = (_mode == Mode::SHAPES) ? fs::path(App::Get()->GetShapesDir()) : fs::path(App::Get()->GetTexturesDir());
    if (_foundFiles.empty())
    {
        if (!fs::is_directory(rootDir)) 
        {
            std::cerr << "Asset directory in settings is not a directory!" << std::endl;
            return;
        }
        
        for (auto const& entry : fs::recursive_directory_iterator{rootDir})
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

            _foundFiles.push_back(entry.path());
        }
    }

    _GetFrames(rootDir);
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
    if (strcmp(_searchFilterBuffer, _searchFilterPrevious) != 0)
    {
        if (_mode == Mode::TEXTURES)
            _GetFrames(fs::path(App::Get()->GetTexturesDir()));
        else if (_mode == Mode::SHAPES)
            _GetFrames(fs::path(App::Get()->GetShapesDir()));
    }
    strcpy(_searchFilterPrevious, _searchFilterBuffer);

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

void PickMode::_DrawGridView(Rectangle framesView) 
{
    const int FRAMES_PER_ROW = (int)framesView.width / FRAME_SPACING;
    Rectangle framesContent = Rectangle{
        .x = 0, 
        .y = 0, 
        .width = framesView.width - 16, 
        .height = ceilf((float)_frames.size() / FRAMES_PER_ROW) * FRAME_SPACING + 64
    };

    Rectangle scissorRect = GuiScrollPanel(framesView, NULL, framesContent, &_scroll);

    // Drawing the scrolling view
    BeginScissorMode(scissorRect.x, scissorRect.y, scissorRect.width, scissorRect.height);
    {
        for (int i = 0; i < _frames.size(); ++i)
        {
            Rectangle rect = Rectangle{
                .x = framesView.x + FRAME_MARGIN + (i % FRAMES_PER_ROW) * FRAME_SPACING + _scroll.x,
                .y = framesView.y + FRAME_MARGIN + (i / FRAMES_PER_ROW) * FRAME_SPACING + _scroll.y,
                .width = FRAME_SIZE,
                .height = FRAME_SIZE};

            if (CheckCollisionRecs(rect, scissorRect))
            {
                _DrawFrame(_frames[i], rect);
            }
        }
    }
    EndScissorMode();
}

void PickMode::_DrawListView(Rectangle framesView)
{
    const int SPACING_WITH_LABEL = FRAME_SPACING + (_longestLabelLength * 10) + 8;
    int framesPerRow = (int)floorf(framesView.width / SPACING_WITH_LABEL);
    if (framesPerRow < 1) framesPerRow = 1;
    Rectangle framesContent = Rectangle{ 
        .x = 0, 
        .y = 0, 
        .width = framesView.width - 16, 
        .height = ceilf((float)_frames.size() / framesPerRow) * FRAME_SPACING + 64 };
    Rectangle scissorRect = GuiScrollPanel(framesView, NULL, framesContent, &_scroll);

    // Drawing the scrolling view
    BeginScissorMode(scissorRect.x, scissorRect.y, scissorRect.width, scissorRect.height);
    {
        for (int i = 0; i < _frames.size(); ++i)
        {
            Rectangle rect = Rectangle{
                .x = framesView.x + FRAME_MARGIN + (i % framesPerRow) * SPACING_WITH_LABEL + _scroll.x,
                .y = framesView.y + FRAME_MARGIN + (i / framesPerRow) * FRAME_SPACING + _scroll.y,
                .width = FRAME_SIZE,
                .height = FRAME_SIZE};
            
            if (CheckCollisionRecs(rect, scissorRect))
            {

                _DrawFrame(_frames[i], rect);

                Rectangle labelRect = Rectangle{
                    .x = rect.x + rect.width + FRAME_MARGIN,
                    .y = rect.y,
                    .width = float(SPACING_WITH_LABEL - FRAME_SPACING),
                    .height = rect.height
                };

                // Allow the label to be clickable as well to select the frame
                if (GuiLabelButton(labelRect, _frames[i].label.c_str())) 
                {
                    _selectedFrame = _frames[i];
                }
            }
        }
    }
    EndScissorMode();
}

void PickMode::_DrawFrame(Frame& frame, Rectangle rect) 
{
    // Handle clicking
    if (CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        _selectedFrame = frame;
    }

    // Black background & outline
    Rectangle outline = Rectangle{rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4};
    DrawRectangle(outline.x, outline.y, outline.width, outline.height, BLACK);
    
    //Draw the rendered texture
    Texture2D tex;
    if (_mode == Mode::SHAPES)
        tex = _GetIcon(frame.filePath).texture;
    else if (_mode == Mode::TEXTURES)
        tex = _GetTexture(frame.filePath);
	Rectangle source = Rectangle { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    DrawTexturePro(tex, source, rect, Vector2Zero(), 0.0f, WHITE);
    
    //White selection outline
    if (_selectedFrame.filePath == frame.filePath) 
        DrawRectangleLinesEx(outline, 2.0f, WHITE); 
}

void PickMode::Draw()
{
    //Draw search box
    GuiLabel(Rectangle{32, UPPER_MARGIN, 128, 32}, "SEARCH:");
    Rectangle searchBoxRect = Rectangle{128, UPPER_MARGIN, (float)GetScreenWidth() / 3.0f, 32};
    if (GuiTextBox(searchBoxRect, _searchFilterBuffer, SEARCH_BUFFER_SIZE, _searchFilterFocused))
    {
        _searchFilterFocused = !_searchFilterFocused;
    }
    
    //Clear button
    Rectangle clearButtonRect = Rectangle{searchBoxRect.x + searchBoxRect.width + 4, searchBoxRect.y, 96, 32};
    if (GuiButton(clearButtonRect, "Clear"))
    {
        memset(_searchFilterBuffer, 0, SEARCH_BUFFER_SIZE * sizeof(char));
    }

    //Viewing types selection
    Rectangle viewToggleRect = Rectangle{
        .x = (float)GetScreenWidth() - 32 - 128, 
        .y = clearButtonRect.y,
        .width = 64,
        .height = clearButtonRect.height 
        };
    _view = (View)GuiToggleGroup(viewToggleRect, "GRID;LIST", (int)_view);

    // Draw views
    Rectangle framesView = Rectangle{32, 96, (float)GetScreenWidth() - 64, (float)GetScreenHeight() - 128};
    if (_view == View::GRID) _DrawGridView(framesView);
    else if (_view == View::LIST) _DrawListView(framesView);

    // Draw the text showing the selected frame's label.
    if (!_selectedFrame.filePath.empty())
    {
        std::string selectString = std::string("Selected: ") + _selectedFrame.label;

        GuiLabel(Rectangle{
                    .x = 64, 
                    .y = (float)GetScreenHeight() - 24, 
                    .width = (float)GetScreenWidth() / 2.0f, 
                    .height = 16},
                selectString.c_str());
    }
}
