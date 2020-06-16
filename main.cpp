#include "app.h"

#ifdef main
#undef main
#endif

void runTests()
{

}

int main(int argc, char* args[])
{
    runTests();

    App app;
    app.beginLoop();
    return 0;
}