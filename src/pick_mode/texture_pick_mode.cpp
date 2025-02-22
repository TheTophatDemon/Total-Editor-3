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

TexturePickMode::TexturePickMode(App::Settings& settings)
    : PickMode(settings, 2, ".png")
{
}

void TexturePickMode::OnEnter()
{
    _rootDir = fs::path(App::Get()->GetTexturesDir());

    PickMode::OnEnter();
}

void TexturePickMode::OnExit()
{
    for (const auto& pair : _loadedTextures)
    {
        UnloadTexture(pair.second);
    }
    _loadedTextures.clear();
}

TexturePickMode::TexSelection TexturePickMode::GetPickedTextures() const
{
    return _selectedTextures;
}

void TexturePickMode::SetPickedTextures(TexturePickMode::TexSelection newTextures)
{
    _selectedTextures = newTextures;
}

Texture TexturePickMode::GetFrameTexture(const fs::path& filePath)
{
    if (_loadedTextures.find(filePath) == _loadedTextures.end())
    {
        Texture2D tex = LoadTexture(filePath.string().c_str());
        _loadedTextures[filePath] = tex;
        return tex;
    }
    return _loadedTextures[filePath];
}

void TexturePickMode::SelectFrame(const Frame frame)
{
    std::shared_ptr<Assets::TexHandle> tex = Assets::GetTexture(frame.filePath);
    
    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) 
    {
        _selectedTextures[0] = tex;
    }
    else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        _selectedTextures[1] = tex;
    }
}

bool TexturePickMode::IsFrameSelected(const fs::path& filePath)
{
    for (const std::shared_ptr<Assets::TexHandle>& tex : _selectedTextures)
    {
        if (tex->GetPath() == filePath) return true;
    }
    return false;
}

std::string TexturePickMode::GetSideLabel(const Frame frame)
{
    bool primary = frame.filePath == _selectedTextures[0]->GetPath();
    bool secondary = frame.filePath == _selectedTextures[1]->GetPath();
    std::stringstream sideLabel;
    if (primary) sideLabel << "(P)\n";
    if (secondary) sideLabel << "(S)";
    return sideLabel.str();
}