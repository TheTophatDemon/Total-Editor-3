#ifndef FILES_UTIL_H
#define FILES_UTIL_H

#include <string>
#include <initializer_list>

std::string BuildPath(std::initializer_list<std::string> components) {
    std::string output;
    for (const std::string& s : components) {
        output += s;
        if (s != *(components.end()-1) && s[s.length() - 1] != '/') {
            output += '/';
        }
    }
    return output;
}

#endif