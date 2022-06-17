#ifndef TEXT_UTIL_H
#define TEXT_UTIL_H

#include "raylib.h"

#include <string>
#include <initializer_list>

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
inline int GetStringWidth(Font font, const std::string &string)
{
    int width = 0;
    for (const char c : string)
    {
        int g = 0;
        for (g = 0; g < font.glyphCount; ++g)
        {
            if (font.glyphs[g].value == (int) c)
            {
                width += font.glyphs[g].offsetX + font.glyphs[g].advanceX;
                break;
            }
        }
        if (g == font.glyphCount)
        {
            //Unknown glyph
            width += font.baseSize;
        }
    }
    return width;
}

#endif