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

#ifndef TEXT_UTIL_H
#define TEXT_UTIL_H

#include "raylib.h"

#include <string>
#include <initializer_list>
#include <filesystem>
namespace fs = std::filesystem;

inline std::string BuildPath(std::initializer_list<std::string> components) {
    std::string output;
    for (const std::string& s : components) {
        output += s;
        if (s != *(components.end()-1) && s[s.length() - 1] != '/') {
            output += '/';
        }
    }
    return output;
}

//Returns the approximate width, in pixels, of a string written in the given font.
//Based off of Raylib's DrawText functions
inline int GetStringWidth(Font font, float fontSize, const std::string &string)
{
    float scaleFactor = fontSize / font.baseSize;
    int maxWidth = 0;
    int lineWidth = 0;
    for (int i = 0; i < string.size();)
    {
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&string[i], &codepointByteCount);
        int g = GetGlyphIndex(font, codepoint);

        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            maxWidth = Max(lineWidth, maxWidth);
            lineWidth = 0;
        }
        else 
        {
            if (font.glyphs[g].advanceX == 0) lineWidth += ((float)font.recs[g].width*scaleFactor);
            else lineWidth += ((float)font.glyphs[g].advanceX*scaleFactor);
        }

        i += codepointByteCount;
    }
    maxWidth = Max(lineWidth, maxWidth);
    return maxWidth;
}

#endif