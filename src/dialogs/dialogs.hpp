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

#ifndef DIALOGS_H
#define DIALOGS_H

#include <cstring>
#include <map>
#include <set>
#include <functional>
#include <initializer_list>
#include <filesystem>
namespace fs = std::filesystem;

#include "../tile.hpp"
#include "../ent.hpp"
#include "../app.hpp"

class Dialog 
{ 
public:
    inline virtual ~Dialog() {}
    //Returns false when the dialog should be closed.
    virtual bool Draw() = 0; 
    inline virtual void Update() {}
};

class NewMapDialog : public Dialog 
{
public: 
    NewMapDialog();
    virtual bool Draw() override;
protected:
    int _mapDims[3];
};

class ExpandMapDialog: public Dialog
{
public:
    ExpandMapDialog();
    virtual bool Draw() override;
protected:
    bool _spinnerActive;
    bool _chooserActive;
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
    // If extensions is left blank, then only directories are selectable.
    FileDialog(std::string title, 
        std::initializer_list<std::string> extensions, 
        std::function<void(std::filesystem::path)> callback, 
        bool writeMode);

    virtual bool Draw() override;
protected:
    //Called when a file has been successfully selected.
    std::string _title;
    std::set<std::string> _extensions;
    std::function<void(fs::path)> _callback;
    fs::path _currentDir, _overwriteDir;

    char _fileNameBuffer[TEXT_FIELD_MAX];
    bool _overwritePromptOpen, _writeMode;
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
    char _texDirBuffer[TEXT_FIELD_MAX];
    char _shapeDirBuffer[TEXT_FIELD_MAX];
    char _defaultTexBuffer[TEXT_FIELD_MAX];
    char _defaultShapeBuffer[TEXT_FIELD_MAX];
    char _hiddenAssetRegexBuffer[TEXT_FIELD_MAX];
    bool _hiddenAssetRegexValid;
    std::unique_ptr<FileDialog> _fileDialog;
};

class SettingsDialog : public Dialog
{
public:
    SettingsDialog(App::Settings &settings);
    virtual bool Draw() override;
protected:
    App::Settings &_settingsOriginal;
    App::Settings _settingsCopy;
};

class AboutDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class ShortcutsDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class InstructionsDialog : public Dialog
{
public:
    virtual bool Draw() override;
};

class ExportDialog : public Dialog
{
public:
    ExportDialog(App::Settings &settings);
    virtual bool Draw() override;
protected:
    App::Settings &_settings;
    std::unique_ptr<FileDialog> _dialog;
    char _filePathBuffer[TEXT_FIELD_MAX];
};

class ConfirmationDialog : public Dialog
{
public:
    ConfirmationDialog(const std::string& titleText, const std::string& bodyText, const std::string& confirmText, 
        const std::string& cancelText, std::function<void(bool)> callback);
    virtual bool Draw() override;
protected:
    std::string _titleText, _bodyText, _confirmText, _cancelText;
    std::function<void(bool)> _callback;
};

#endif
