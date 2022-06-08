#ifndef ARRAY_LIST_H
#define ARRAY_LIST_H

#include "raylib.h"
#include <stdlib.h>

#define DECLARE_LIST(TYPE) \
    typedef struct { \
        TYPE *data; \
        size_t length; \
        size_t capacity; \
    } TYPE ## List;

DECLARE_LIST(Model);
DECLARE_LIST(Material);
DECLARE_LIST(Matrix);

#define LIST_APPEND(TYPE, LIST, ELEM) \
    { \
        if ((LIST).length >= (LIST).capacity || (LIST).capacity == 0) \
        { \
            if ((LIST).capacity == 0) (LIST).capacity = 1; \
            (LIST).capacity *= 2; \
            (LIST).data = realloc((void *)(LIST).data, sizeof(TYPE) * (LIST).capacity); \
        } \
        (LIST).data[(LIST).length++] = ELEM;\
    }

#define LIST_RESERVE(TYPE, LIST, CAPACITY) { \
    (LIST).capacity = (CAPACITY) ;  \
    (LIST).data = realloc((void *)(LIST).data, sizeof(TYPE) * (LIST).capacity); \
    }

#endif