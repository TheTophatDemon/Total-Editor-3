#ifndef C_HELPERS_H
#define C_HELPERS_H

#include <stdlib.h>

// This prevents me from mixing types in the malloc statements...
#define SAFE_MALLOC(TYPE, COUNT) (TYPE*) malloc((COUNT) * sizeof(TYPE))

#endif