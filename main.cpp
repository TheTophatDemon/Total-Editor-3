#include "app.h"

#ifdef main
#undef main
#endif

int main(int argc, char* args[])
{
    App app;
    app.beginLoop();
    return 0;
}