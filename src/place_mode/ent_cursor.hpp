#pragma once
#ifndef ENT_CURSOR_H
#define ENT_CURSOR_H

#include "cursor.hpp"

#include "../ent.hpp"

struct EntCursor : public Cursor
{
	Ent ent;

	EntCursor();
	void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) override;
	void Draw(MapMan& mapMan) override;
};

#endif