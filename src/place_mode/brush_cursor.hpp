#pragma once
#ifndef BRUSH_CURSOR_H
#define BRUSH_CURSOR_H

#include "cursor.hpp"

#include "../tile.hpp"

struct BrushCursor : public Cursor
{
	TileGrid brush;

	BrushCursor(MapMan& mapMan);
	void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) override;
	void Draw(MapMan& mapMan) override;
};

#endif