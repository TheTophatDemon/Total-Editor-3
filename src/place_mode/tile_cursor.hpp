#pragma once
#ifndef TILE_CURSOR_H
#define TILE_CURSOR_H

#include "cursor.hpp"

#include <memory>
#include <array>

#include "../assets.hpp"
#include "../tile.hpp"

struct TileCursor : public Cursor
{
	std::shared_ptr<Assets::ModelHandle> model;
	std::array<std::shared_ptr<Assets::TexHandle>, TEXTURES_PER_TILE> textures;
	std::array<Material, TEXTURES_PER_TILE> materials;
	uint8_t yaw, pitch;

	TileCursor();
	~TileCursor();
	void Update(MapMan& mapMan, size_t i, size_t j, size_t k, size_t w, size_t h, size_t l) override;
	void Draw(MapMan& mapMan) override;
};

#endif