#include "pick_mode.hpp"

#include "extras/raygui.h"
#include "raymath.h"

#include <cstring>

#include "assets.hpp"

const int FRAME_SIZE = 64;
const int FRAME_MARGIN = 16;
const int FRAME_SPACING = (FRAME_SIZE + FRAME_MARGIN * 2);

#define SEARCH_BUFFER_SIZE 256

PickMode::PickMode(AppContext *context, Mode mode)
    : _mode(mode),
    _context(context),
    _scroll(Vector2Zero()),
    _selectedFrame(nullptr),
    _searchFilterFocused(false)
{
    
    _searchFilterBuffer = (char *)calloc(SEARCH_BUFFER_SIZE, sizeof(char));
}

PickMode::~PickMode() {
    free(_searchFilterBuffer);
}

void PickMode::OnEnter() {

    _frames.clear();

    //Retrieve files and generate frames for each
    char **files = nullptr;
    int fileCount = 0;
    switch (_mode) {
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
            frame.label = std::string(fileName);
            switch (_mode) {
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

    ClearDirectoryFiles();
}

void PickMode::OnExit() {

}

void PickMode::Update() 
{
    if (_selectedFrame) {
        _context->selectedTexture = _selectedFrame->tex;
    }
}

void PickMode::Draw() 
{
    Rectangle _framesView = (Rectangle) { 32, 96, _context->screenWidth - 64, _context->screenHeight - 128 };
    const int FRAMES_PER_ROW = (int)_framesView.width / FRAME_SPACING;
    Rectangle _framesContent = (Rectangle){0, 0, _framesView.width - 16, ceilf((float)_frames.size() / FRAMES_PER_ROW) * FRAME_SPACING + 64};

    GuiLabel((Rectangle){32, 32, 128, 32}, "SEARCH:");
    Rectangle searchBoxRect = (Rectangle){ 128, 32, _context->screenWidth / 3.0f, 32 }; 
    if (GuiTextBox(searchBoxRect, _searchFilterBuffer, SEARCH_BUFFER_SIZE, _searchFilterFocused)) {
        _searchFilterFocused = !_searchFilterFocused;
    }

    Rectangle scissorRect = GuiScrollPanel(_framesView, NULL, _framesContent, &_scroll);

    BeginScissorMode(scissorRect.x, scissorRect.y, scissorRect.width, scissorRect.height);
    {
        //Position frames based on order and search filter.
        int f = 0;
        for (Frame& frame : _frames) {
            std::string lowerCaseLabel = TextToLower(frame.label.c_str());
            if (strlen(_searchFilterBuffer) == 0 || lowerCaseLabel.find(TextToLower(_searchFilterBuffer)) != std::string::npos) 
            {
                Rectangle rect = (Rectangle) { 
                    .x = _framesView.x + FRAME_MARGIN + (f % FRAMES_PER_ROW) * FRAME_SPACING + _scroll.x,
                    .y = _framesView.y + FRAME_MARGIN + (f / FRAMES_PER_ROW) * FRAME_SPACING + _scroll.y,
                    .width = FRAME_SIZE, .height = FRAME_SIZE
                };
                ++f;

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

    if (_selectedFrame) {
        Rectangle previewRect = (Rectangle) {
            searchBoxRect.x + searchBoxRect.width + 16, 16, FRAME_SIZE, FRAME_SIZE
        };
        DrawTextureQuad(*_selectedFrame->tex, Vector2One(), Vector2Zero(), previewRect, WHITE);
        DrawRectangleLinesEx((Rectangle){previewRect.x - 2, previewRect.y - 2, previewRect.width + 4, previewRect.height + 4}, 2.0f, WHITE);
        std::string selectString = std::string("Selected: \n") + _selectedFrame->label;
        
        GuiLabel((Rectangle) {
            previewRect.x + previewRect.width + 8, 16, _context->screenWidth - 32 - (previewRect.x + previewRect.width + 8), FRAME_SIZE
        }, selectString.c_str());
    }
}