/**
 * Copyright (c) 2022 Alexander Lunsford
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

#include "assets.hpp"
#include "text_util.hpp"

const int FRAME_SIZE = 64;
const int FRAME_MARGIN = 16;
const int FRAME_SPACING = (FRAME_SIZE + FRAME_MARGIN * 2);

#define UPPER_MARGIN 48

PickMode::PickMode(Mode mode)
    : _mode(mode),
      _scroll(Vector2Zero()),
      _selectedFrame(nullptr),
      _searchFilterFocused(false),
      _view(mode == Mode::TEXTURES ? View::GRID : View::LIST),
      _longestLabelLength(0)
{
    memset(_searchFilterBuffer, 0, sizeof(char) * SEARCH_BUFFER_SIZE);
}

void PickMode::_GetFrames(fs::path rootDir)
{
    if (!fs::is_directory(rootDir)) 
    {
        std::cerr << "Asset directory in settings is not a directory!" << std::endl;
        return;
    }
    
    for (auto const& entry : fs::recursive_directory_iterator{rootDir})
    {
        if (entry.is_directory() || !entry.is_regular_file()) continue;

        auto labelPath = fs::relative(entry.path(), rootDir);
        auto ext = labelPath.extension().string();
        if (ext == ".png")
        {
            Frame frame = {
                .tex = Assets::GetTexture(entry.path()),
                .shape = nullptr,
                .label = labelPath.string()
            };
            _frames.push_back(frame);
        }
        else if (ext == ".obj")
        {
            Frame frame = {
                .tex = nullptr,
                .shape = Assets::GetModel(entry.path()),
                .renderTex = LoadRenderTexture(FRAME_SIZE, FRAME_SIZE),
                .label = labelPath.string(),
            };
            _frames.push_back(frame);
        }

        //Update longest label length
        _longestLabelLength = Max(_longestLabelLength, labelPath.string().length());
    }
}

void PickMode::OnEnter()
{
    _selectedFrame = nullptr;
    _frames.clear();

    if (_mode == Mode::TEXTURES)
        _GetFrames(fs::path(App::Get()->GetTexturesDir()));
    else if (_mode == Mode::SHAPES)
        _GetFrames(fs::path(App::Get()->GetShapesDir()));
}

void PickMode::OnExit()
{
}

void PickMode::Update()
{

    _filteredFrames.clear();
    for (Frame& frame : _frames) 
    {
        //Filter frames by the search text. If the search text is contained anywhere in the file path, then it passes.
        std::string lowerCaseLabel = TextToLower(frame.label.c_str());
        if (strlen(_searchFilterBuffer) == 0 || lowerCaseLabel.find(TextToLower(_searchFilterBuffer)) != std::string::npos)
        {
            _filteredFrames.push_back(&frame);
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
        .height = ceilf((float)_filteredFrames.size() / FRAMES_PER_ROW) * FRAME_SPACING + 64
    };

    Rectangle scissorRect = GuiScrollPanel(framesView, NULL, framesContent, &_scroll);

    // Drawing the scrolling view
    BeginScissorMode(scissorRect.x, scissorRect.y, scissorRect.width, scissorRect.height);
    {
        int f = 0;
        for (Frame *frame : _filteredFrames)
        {
            Rectangle rect = Rectangle{
                .x = framesView.x + FRAME_MARGIN + (f % FRAMES_PER_ROW) * FRAME_SPACING + _scroll.x,
                .y = framesView.y + FRAME_MARGIN + (f / FRAMES_PER_ROW) * FRAME_SPACING + _scroll.y,
                .width = FRAME_SIZE,
                .height = FRAME_SIZE};

            ++f;

            _DrawFrame(frame, rect);
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
        .height = ceilf((float)_filteredFrames.size() / framesPerRow) * FRAME_SPACING + 64 };
    Rectangle scissorRect = GuiScrollPanel(framesView, NULL, framesContent, &_scroll);

    // Drawing the scrolling view
    BeginScissorMode(scissorRect.x, scissorRect.y, scissorRect.width, scissorRect.height);
    {
        int f = 0;
        for (Frame *frame : _filteredFrames)
        {
            Rectangle rect = Rectangle{
                .x = framesView.x + FRAME_MARGIN + (f % framesPerRow) * SPACING_WITH_LABEL + _scroll.x,
                .y = framesView.y + FRAME_MARGIN + (f / framesPerRow) * FRAME_SPACING + _scroll.y,
                .width = FRAME_SIZE,
                .height = FRAME_SIZE};

            ++f;

            _DrawFrame(frame, rect);

            Rectangle labelRect = Rectangle{
                .x = rect.x + rect.width + FRAME_MARGIN,
                .y = rect.y,
                .width = float(SPACING_WITH_LABEL - FRAME_SPACING),
                .height = rect.height
            };
            if (GuiLabelButton(labelRect, frame->label.c_str())) 
            {
                _selectedFrame = frame;
            }
        }
    }
    EndScissorMode();
}

void PickMode::_DrawFrame(Frame *frame, Rectangle rect) 
{
    //Camera for the model preview icons
    static Camera camera = Camera {
        .position = Vector3 { 4.0f, 4.0f, 4.0f },
        .target = Vector3Zero(),
        .up = Vector3 { 0.0f, -1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE
    };

    if (CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        _selectedFrame = frame;
    }

    Rectangle outline = Rectangle{rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4};
    DrawRectangle(outline.x, outline.y, outline.width, outline.height, BLACK); //Black background
    
    //Texture
    Texture2D tex;
    if (_mode == Mode::SHAPES)
    {
        //Update/redraw the shape preview icons so that they spin
        BeginTextureMode(frame->renderTex);
        ClearBackground(BLACK);
        BeginMode3D(camera);

        DrawModelWiresEx(frame->shape->GetModel(), Vector3Zero(), Vector3{0.0f, 1.0f, 0.0f}, float(GetTime() * 180.0f), Vector3One(), GREEN);

        EndMode3D();
        EndTextureMode();

        tex = frame->renderTex.texture;
    }
    else
    {
        tex = frame->tex->GetTexture();
    }
	Rectangle source = Rectangle { 0.0f, 0.0f, (float)tex.width, (float)tex.height };
    DrawTexturePro(tex, source, rect, Vector2Zero(), 0.0f, WHITE);
    
    if (_selectedFrame == frame) DrawRectangleLinesEx(outline, 2.0f, WHITE); //White selection outline
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

    Rectangle framesView = Rectangle{32, 96, (float)GetScreenWidth() - 64, (float)GetScreenHeight() - 128};
    if (_view == View::GRID) _DrawGridView(framesView);
    else if (_view == View::LIST) _DrawListView(framesView);

    // Draw the text showing the selected frame's label.
    if (_selectedFrame)
    {
        std::string selectString = std::string("Selected: ") + _selectedFrame->label;

        GuiLabel(Rectangle{
                    .x = 64, 
                    .y = (float)GetScreenHeight() - 24, 
                    .width = (float)GetScreenWidth() / 2.0f, 
                    .height = 16},
                selectString.c_str());
    }
}
