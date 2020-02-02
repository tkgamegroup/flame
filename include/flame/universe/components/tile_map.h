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

		cTileMap() :
			Component("cTileMap")
		{
		}

		FLAME_UNIVERSE_EXPORTS uint tile_count() const;
		FLAME_UNIVERSE_EXPORTS uint tile(uint idx) const;
		FLAME_UNIVERSE_EXPORTS void add_tile(uint id);
		FLAME_UNIVERSE_EXPORTS void remove_tile(uint id);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2u& s);
		FLAME_UNIVERSE_EXPORTS int cell(const Vec2i& idx) const;
		FLAME_UNIVERSE_EXPORTS void set_cell(const Vec2u& idx, int tile_idx /* -1 is null */);
		FLAME_UNIVERSE_EXPORTS void clear_cells();

		FLAME_UNIVERSE_EXPORTS static cTileMap* create();
	};
}
