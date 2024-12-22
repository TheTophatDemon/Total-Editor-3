#ifndef DEFER_H
#define DEFER_H

#include <functional>

// Calls a statement at the end of the enclosing scope.
#define DEFER_PASTE_HELPER(a,b) a ## b
#define DEFER_PASTE(a,b) DEFER_PASTE_HELPER(a,b)

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