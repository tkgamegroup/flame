#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cTileMap : Component
	{
		cElement* element;

		Vec2u size_;
		Vec2f cell_size;
		Vec4c color;

		cTileMap() :
			Component("cTileMap")
		{
		}

		FLAME_UNIVERSE_EXPORTS const std::vector<uint>& tiles() const;
		FLAME_UNIVERSE_EXPORTS void add_tile(uint id);
		FLAME_UNIVERSE_EXPORTS void remove_tile(uint id);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2u& s);
		FLAME_UNIVERSE_EXPORTS int cell(const Vec2u& idx) const;
		FLAME_UNIVERSE_EXPORTS void set_cell(const Vec2u& idx, int tile_idx /* -1 is null */);

		FLAME_UNIVERSE_EXPORTS static cTileMap* create();
	};
}
