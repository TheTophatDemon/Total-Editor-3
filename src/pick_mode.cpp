#include "pick_mode.hpp"

#include "extras/raygui.h"
#include "raymath.h"

#include <cstring>
#include <stack>

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
}

void PickMode::_GetFrames(std::string rootDir)
{
    std::stack<std::string> dirs;
    dirs.push(rootDir);

    _longestLabelLength = 0;
    while (!dirs.empty())
    {
        std::string dir = dirs.top();
        dirs.pop();

        char **files = nullptr;
        int fileCount = 0;
        //This function will free the memory stored in `files` every time it is called, so be careful not to call it during iteration.
        files = GetDirectoryFiles(dir.c_str(), &fileCount);

        for (int f = 0; f < fileCount; ++f)
        {
            std::string fullPath = BuildPath({dir, files[f]});
            if (DirectoryExists(fullPath.c_str()) && strcmp(files[f], ".") != 0 && strcmp(files[f], "..") != 0)
            {
                dirs.push(fullPath);
            }
            else if (_mode == Mode::TEXTURES && IsFileExtension(files[f], ".png"))
            {
                Frame frame = {
                    .tex = Assets::GetTexture(fullPath),
                    .shape = nullptr,
                    .label = fullPath};
                _frames.push_back(frame);
            }
            else if (_mode == Mode::SHAPES && IsFileExtension(files[f], ".obj"))
            {
                Model *shape = Assets::GetShape(fullPath);
                Frame frame = {
                    .tex = Assets::GetShapeIcon(shape),
                    .shape = shape,
                    .label = fullPath};
                _frames.push_back(frame);
            }

            if (fullPath.length() > _longestLabelLength) _longestLabelLength = fullPath.length();
        }
    }
}

void PickMode::OnEnter()
{
    _selectedFrame = nullptr;
    _frames.clear();

    if (_mode == Mode::TEXTURES)
        _GetFrames(App::Get()->GetTexturesDir());
    else if (_mode == Mode::SHAPES)
        _GetFrames(App::Get()->GetShapesDir());

    ClearDirectoryFiles();
}

void PickMode::OnExit()
{
}

void PickMode::Update()
{
    //Filter frames by the search text. If the search text is contained anywhere in the file path, then it passes.
    _filteredFrames.clear();
    for (Frame& frame : _frames) {
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
    Rectangle framesContent = (Rectangle){
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
            Rectangle rect = (Rectangle){
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
    Rectangle framesContent = (Rectangle){ 
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
            Rectangle rect = (Rectangle){
                .x = framesView.x + FRAME_MARGIN + (f % framesPerRow) * SPACING_WITH_LABEL + _scroll.x,
                .y = framesView.y + FRAME_MARGIN + (f / framesPerRow) * FRAME_SPACING + _scroll.y,
                .width = FRAME_SIZE,
                .height = FRAME_SIZE};

            ++f;

            _DrawFrame(frame, rect);

            Rectangle labelRect = (Rectangle){
                .x = rect.x + rect.width + FRAME_MARGIN,
                .y = rect.y,
                .width = SPACING_WITH_LABEL - FRAME_SPACING,
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
    if (CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        _selectedFrame = frame;
    }

    Rectangle outline = (Rectangle){rect.x - 2, rect.y - 2, rect.width + 4, rect.height + 4};
    DrawRectangle(outline.x, outline.y, outline.width, outline.height, BLACK); //Black background
    DrawTextureQuad(*(frame->tex), Vector2One(), Vector2Zero(), rect, WHITE); //Texture
    if (_selectedFrame == frame) DrawRectangleLinesEx(outline, 2.0f, WHITE); //White selection outline
}

void PickMode::Draw()
{
    //Draw search box
    GuiLabel((Rectangle){32, UPPER_MARGIN, 128, 32}, "SEARCH:");
    Rectangle searchBoxRect = (Rectangle){128, UPPER_MARGIN, (float)GetScreenWidth() / 3.0f, 32};
    if (GuiTextBox(searchBoxRect, _searchFilterBuffer, SEARCH_BUFFER_SIZE, _searchFilterFocused))
    {
        _searchFilterFocused = !_searchFilterFocused;
    }
    
    //Clear button
    Rectangle clearButtonRect = (Rectangle){searchBoxRect.x + searchBoxRect.width + 4, searchBoxRect.y, 96, 32};
    if (GuiButton(clearButtonRect, "Clear"))
    {
        memset(_searchFilterBuffer, 0, SEARCH_BUFFER_SIZE * sizeof(char));
    }
    //Viewing types selection
    Rectangle viewToggleRect = (Rectangle){
        .x = (float)GetScreenWidth() - 32 - 128, 
        .y = clearButtonRect.y,
        .width = 64,
        .height = clearButtonRect.height 
        };
    _view = (View)GuiToggleGroup(viewToggleRect, "GRID;LIST", (int)_view);

    Rectangle framesView = (Rectangle){32, 96, (float)GetScreenWidth() - 64, (float)GetScreenHeight() - 128};
    if (_view == View::GRID) _DrawGridView(framesView);
    else if (_view == View::LIST) _DrawListView(framesView);

    // Draw the text showing the selected frame's label.
    if (_selectedFrame)
    {
        std::string selectString = std::string("Selected: ") + _selectedFrame->label;

        GuiLabel((Rectangle){
                    .x = 64, 
                    .y = (float)GetScreenHeight() - 24, 
                    .width = (float)GetScreenWidth() / 2.0f, 
                    .height = 16},
                selectString.c_str());
    }
}