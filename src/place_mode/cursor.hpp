#pragma once
#ifndef CURSOR_H
#define CURSOR_H

#include <stdlib.h>
#include "raylib.h"
#include "../map_man/map_man.hpp"

struct Cursor 
{
	Vector3 position;
	Vector3 endPosition;
	bool disableBorder;

	virtual void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) = 0;
	virtual void Draw(MapMan& mapMan) = 0;
};
#endif