#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTileMap : Component
	{
		inline static auto type_name = "flame::cTileMap";
		inline static auto type_hash = ch(type_name);

		cTileMap() :
			Component(type_name, type_hash)
		{
		}

		virtual int get_res_id() const = 0;
		virtual void set_res_id(int id) = 0;

		virtual uvec2 get_cell_count() const = 0;
		virtual void set_cell_count(const uvec2& c) = 0;
		virtual vec2 get_cell_size() const = 0;
		virtual void set_cell_size(const vec2& s) = 0;

		virtual void get_cell(const uvec2& idx, int& tile_id, cvec4& color) const = 0;
		virtual void set_cell(const uvec2& idx, int tile_id, const cvec4 color) = 0;

		virtual bool get_clipping() const = 0;
		virtual void set_clipping(bool c) = 0;

		FLAME_UNIVERSE_EXPORTS static cTileMap* create();
	};
}
