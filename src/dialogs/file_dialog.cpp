#include "dialogs.hpp"

#include "imgui/imgui.h"

FileDialog::FileDialog(std::string title, 
    std::initializer_list<std::string> extensions, 
    std::function<void(std::filesystem::path)> callback, 
    bool writeMode
) : _title(title),
      _extensions(extensions),
      _callback(callback),
      _currentDir(fs::current_path()),
      _overwritePromptOpen(false),
      _writeMode(writeMode)
{
    memset(&_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
}

bool FileDialog::Draw()
{
    bool open = true;
    if (!_overwritePromptOpen) ImGui::OpenPopup(_title.c_str());
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal(_title.c_str(), &open, ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Parent directory button & other controls
        
        if (ImGui::Button("Parent directory") && _currentDir.has_parent_path())
        {
            _currentDir = _currentDir.parent_path();
        }

        if (ImGui::BeginListBox("Files", ImVec2(504.0f, 350.0f)))
        {
            // File list
            fs::directory_iterator fileIter;
            try 
            {
                fileIter = fs::directory_iterator{_currentDir};
            }
            catch (...)
            {
                // For some reason, the directory iterator will show special system folders that aren't even accessible in the file explorer.
                // Navigating into such a folder causes a crash, which I am preventing with this try/catch block.
                // So, going into these folders will just show nothing instead of crashing.
            }

            // Store found files and directories in these sets so that they get automatically sorted (and we can put directories in front of files)
            std::set<fs::path> foundDirectories;
            std::set<fs::path> foundFiles;

            // Get files/directories from the file system
            std::error_code osError;
            for (auto i = fs::begin(fileIter); i != fs::end(fileIter); i = i.increment(osError))
            {
                if (osError) break;
                auto entry = *i;
                if (entry.is_directory())
                {
                    foundDirectories.insert(entry.path());
                }
                if (entry.is_regular_file() && _extensions.find(entry.path().extension().string()) != _extensions.end())
                {
                    foundFiles.insert(entry.path());
                }
            }

            // Display the directories
            for (fs::path entry : foundDirectories)
            {
                std::string entry_str = std::string("[") + entry.stem().string() + "]";
                // Directories are yellow
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
                
                if (ImGui::Selectable(entry_str.c_str()))
                {
                    _currentDir = entry;
                    memset(_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
                }
                ImGui::PopStyleColor();
            }

            // Display the files
            for (fs::path entry : foundFiles)
            {
                std::string entry_str = entry.filename().string();
                // Files are white
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

                if (ImGui::Selectable(entry_str.c_str()))
                {
                    strcpy(_fileNameBuffer, entry_str.c_str());
                }
                ImGui::PopStyleColor();
            }

            ImGui::EndListBox();
        }

        ImGui::InputText("File name", _fileNameBuffer, TEXT_FIELD_MAX);
        
        if (ImGui::Button("SELECT") && (strlen(_fileNameBuffer) > 0 || _extensions.empty()))
        {
            fs::path newPath = _currentDir / _fileNameBuffer;
            if (_writeMode && fs::exists(newPath))
            {
                _overwritePromptOpen = true;
                _overwriteDir = newPath;
            }
            else
            {
                _callback(newPath);

                ImGui::EndPopup();
                return false;
            }
        }

        ImGui::EndPopup();
    }

    if (_overwritePromptOpen) ImGui::OpenPopup("CONFIRM OVERWRITE?");
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("CONFIRM OVERWRITE?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(_overwriteDir.string().c_str());
        ImGui::TextUnformatted("Do you DARE overwrite this file?");
        
        if (ImGui::Button("YES"))
        {
            _callback(_overwriteDir);

            ImGui::EndPopup();
            return false;
        }
        ImGui::SameLine();
        if (ImGui::Button("NO"))
        {
            ImGui::CloseCurrentPopup();
            _overwritePromptOpen = false;
        }

        ImGui::EndPopup();
    }

    return open;
}