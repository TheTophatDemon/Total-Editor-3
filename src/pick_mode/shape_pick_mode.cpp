/**
 * Copyright (c) 2022-present Alexander Lunsford
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and  to alter it and redistribute it
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

ShapePickMode::ShapePickMode(App::Settings& settings)
    : PickMode(settings, 1, ".obj")
{
    _iconCamera = Camera {
        .position = Vector3 { 4.0f, 4.0f, 4.0f },
        .target = Vector3Zero(),
        .up = Vector3 { 0.0f, -1.0f, 0.0f },
        .fovy = 45.0f,
        .projection = CAMERA_PERSPECTIVE,
    };
}

void ShapePickMode::OnEnter()
{
    _rootDir = fs::path(App::Get()->GetShapesDir());

    PickMode::OnEnter();
}

void ShapePickMode::Update()
{
    for (const Frame& frame : _frames) 
    {
        // Update/redraw the shape preview icons so that they spin.
        // This has to be done before the main application renders or it won't work.
        BeginTextureMode(_GetShapeIcon(frame.filePath));
        ClearBackground(BLACK);
        BeginMode3D(_iconCamera);

        DrawModelWiresEx(_GetModel(frame.filePath), Vector3Zero(), Vector3{0.0f, 1.0f, 0.0f}, float(GetTime() * 180.0f), Vector3One(), GREEN);

        EndMode3D();
        EndTextureMode();
    }

    PickMode::Update();
}

void ShapePickMode::OnExit()
{
    for (const auto& pair : _loadedModels)
    {
        UnloadModel(pair.second);
    }
    _loadedModels.clear();

    for (const auto& pair : _loadedIcons)
    {
        UnloadRenderTexture(pair.second);
    }
    _loadedIcons.clear();
}

std::shared_ptr<Assets::ModelHandle> ShapePickMode::GetPickedShape() const
{
    return _selectedShape;
}

void ShapePickMode::SetPickedShape(std::shared_ptr<Assets::ModelHandle> newModel)
{
    _selectedShape = newModel;
}

Texture ShapePickMode::GetFrameTexture(const fs::path& filePath)
{
    return _GetShapeIcon(filePath).texture;
}

void ShapePickMode::SelectFrame(const Frame frame)
{
    _selectedShape = Assets::GetModel(frame.filePath);
}

bool ShapePickMode::IsFrameSelected(const fs::path& filePath)
{
    return _selectedShape->GetPath() == filePath;
}

Model ShapePickMode::_GetModel(const fs::path path)
{
    if (_loadedModels.find(path) == _loadedModels.end())
    {
        Model model = LoadModel(path.string().c_str());
        _loadedModels[path] = model;
        return model;
    }
    return _loadedModels[path];
}

RenderTexture2D ShapePickMode::_GetShapeIcon(const fs::path path)
{
    if (_loadedIcons.find(path) == _loadedIcons.end())
    {
        RenderTexture2D icon = LoadRenderTexture(ICON_SIZE, ICON_SIZE);
        _loadedIcons[path] = icon;
        return icon;
    }
    return _loadedIcons[path];
}