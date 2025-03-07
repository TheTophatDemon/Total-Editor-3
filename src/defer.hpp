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

#ifndef DEFER_H
#define DEFER_H

#include <functional>

#define DEFER_PASTE_HELPER(a,b) a ## b
#define DEFER_PASTE(a,b) DEFER_PASTE_HELPER(a,b)

// Calls a statement at the end of the enclosing scope.
#define DEFER($stmt) DeferWrapper DEFER_PASTE(__defer,__LINE__)([=](){ $stmt; })

class DeferWrapper {
public:
    inline DeferWrapper(const std::function<void()> statement) 
    {
        _statement = statement;
    }
    inline ~DeferWrapper()
    {
        _statement();
    }
private:
    std::function<void()> _statement;
};

#endif