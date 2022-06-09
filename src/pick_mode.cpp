#include "pick_mode.hpp"

#include "extras/raygui.h"
#include "raymath.h"

#include "assets.hpp"

const int FRAME_SIZE = 64;
const int FRAME_MARGIN = 16;
const int FRAME_SPACING = (FRAME_SIZE + FRAME_MARGIN * 2);

PickMode::PickMode(AppContext *context, Mode mode)
    : _mode(mode),
    _context(context),
    _scroll(Vector2Zero()),
    _selectedFrame(nullptr)
{
    _framesView = (Rectangle) { 32, 96, context->screenWidth - 64, context->screenHeight - 128 };

    //Retrieve files and generate frames for each
    char **files = nullptr;
    int fileCount = 0;
    switch (mode) {
    case Mode::TEXTURES:
        files = GetDirectoryFiles("assets/textures/", &fileCount);
        break;
    case Mode::SHAPES:
        files = GetDirectoryFiles("assets/models/shapes/", &fileCount);
        break;
    }

    for (int f = 0; f < fileCount; ++f) {
        if (IsFileExtension(files[f], ".png") || IsFileExtension(files[f], ".obj")) {
            Frame frame;
            const char *fileName = GetFileNameWithoutExt(files[f]);
            frame.label = std::string(TextToLower(fileName));
            switch (mode) {
            case Mode::TEXTURES:
                frame.tex = Assets::GetTexture(std::string("assets/textures/") + files[f]);
                break;
            case Mode::SHAPES:
                //TODO: Generate icons for shape files.
                break;
            }
            _frames.push_back(frame);
        }
    }

    _framesContent = (Rectangle){0, 0, _framesView.width - 16, _frames.size() * FRAME_SPACING + 64};

    ClearDirectoryFiles();
}

void PickMode::Update() 
{
    if (_selectedFrame) {
        _context->selectedTexture = _selectedFrame->tex;
    }
}

void PickMode::Draw() 
{
    Rectangle scissorRect = GuiScrollPanel(_framesView, NULL, _framesContent, &_scroll);

    BeginScissorMode(scissorRect.x, scissorRect.y, scissorRect.width, scissorRect.height);
    {
        //Position frames based on order and search filter.
        const int FRAMES_PER_ROW = (int)_framesView.width / FRAME_SPACING;
        for (int f = 0; f < _frames.size(); ++f) {
            Frame& frame = _frames[f];
            if (_searchFilter.empty() || _searchFilter.find(frame.label) != std::string::npos) {
                Rectangle rect = (Rectangle) { 
                    .x = _framesView.x + FRAME_MARGIN + (f % FRAMES_PER_ROW) * FRAME_SPACING + _scroll.x,
                    .y = _framesView.y + FRAME_MARGIN + (f / FRAMES_PER_ROW) * FRAME_SPACING + _scroll.y,
                    .width = FRAME_SIZE, .height = FRAME_SIZE
                };

                if (CheckCollisionPointRec(GetMousePosition(), scissorRect)
                     && CheckCollisionPointRec(GetMousePosition(), rect) 
                     && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) 
                {
                    _selectedFrame = &frame;
                }

                DrawRectangle(rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4, BLACK);
                DrawTextureQuad(*frame.tex, Vector2One(), Vector2Zero(), rect, WHITE);
                DrawRectangleLinesEx((Rectangle){rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4}, 2.0f, _selectedFrame == &frame ? WHITE : BLACK);
            }
        }
    }
    EndScissorMode();
}