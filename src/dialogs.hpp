/*
Copyright (C) 2022 Alexander Lunsford
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef DIALOGS_H
#define DIALOGS_H

#include <cstring>
#include <map>
#include <set>
#include <functional>
#include <initializer_list>
#include <filesystem>
namespace fs = std::filesystem;

#include "tile.hpp"
#include "ent.hpp"
#include "app.hpp"

#define TEXT_FIELD_MAX 512

class Dialog 
{ 
public:
    inline virtual ~Dialog() {}
    //Returns false when the dialog should be closed.
    virtual bool Draw() = 0; 
};

class NewMapDialog : public Dialog 
{
public: 
    NewMapDialog();
    NewMapDialog(int width, int height, int length);
    virtual bool Draw() override;
protected:
    inline static constexpr int NUM_SPINNERS = 3;
    int _width;
    int _height;
    int _length;
    bool _spinnerActive[NUM_SPINNERS];
};

class ExpandMapDialog: public Dialog
{
public:
    ExpandMapDialog();
    virtual bool Draw() override;
protected:
    bool _chooserActive;
    bool _spinnerActive;
    int _amount;
    Direction _direction;
};

class ShrinkMapDialog: public Dialog
{
public:
    virtual bool Draw() override;
};

class FileDialog : public Dialog
{
public:
    inline FileDialog(std::string title, std::initializer_list<std::string> extensions, std::function<void(std::filesystem::path)> callback) 
        : _title(title),
          _extensions(extensions),
          _callback(callback),
          _currentDir(fs::current_path()),
          _scroll(Vector2Zero()),
          _fileNameEdit(false)
    {
        memset(&_fileNameBuffer, 0, sizeof(char) * TEXT_FIELD_MAX);
    }

    inline virtual ~FileDialog() override 
    {
    }

    virtual bool Draw() override;
protected:
    //Called when a file has been successfully selected.
    std::function<void(fs::path)> _callback;
    std::string _title;
    std::set<std::string> _extensions;
    fs::path _currentDir;
    Vector2 _scroll;

    char _fileNameBuffer[TEXT_FIELD_MAX];
    bool _fileNameEdit;
};

class CloseDialog : public Dialog
{
public:
    CloseDialog();
    virtual bool Draw() override;
protected:
    int _messageIdx;
};

class AssetPathDialog : public Dialog
{
public:
    AssetPathDialog(App::Settings &settings);
    virtual bool Draw() override;
protected:
    App::Settings &_settings;
    char _texPathBuffer[TEXT_FIELD_MAX];
    char _shapePathBuffer[TEXT_FIELD_MAX];
    bool _texPathEdit;
    bool _shapePathEdit;
};

class SettingsDialog : public Dialog
{
public:
    SettingsDialog(App::Settings &settings);
    virtual bool Draw() override;
protected:
    App::Settings &_settings;
    int _undoMax;
    bool _undoMaxEdit;
    float _sensitivity;
};

class AboutDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class ShortcutsDialog : public Dialog
{
public:
    ShortcutsDialog();
    virtual bool Draw() override;
protected:
    Vector2 _scroll;
};

class InstructionsDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class ExportDialog : public Dialog
{
public:
    ExportDialog();
    virtual bool Draw() override;
protected:
    std::unique_ptr<FileDialog> _dialog;
    char _filePath[TEXT_FIELD_MAX];
    bool _filePathEdit;
    bool _separateGeometry;
};

#endif