#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cTileMap : Component  // R !ctor !dtor !type_name !type_hash
	{
		inline static auto type_name = "flame::cTileMap";
		inline static auto type_hash = ch(type_name);

		cTileMap() :
			Component(type_name, type_hash)
		{
		}

		virtual int get_res_id() const = 0;
		virtual void set_res_id(int id) = 0;

		virtual Vec2u get_cell_count() const = 0;
		virtual void set_cell_count(const Vec2u& c) = 0;
		virtual Vec2f get_cell_size() const = 0;
		virtual void set_cell_size(const Vec2f& s) = 0;

		virtual void get_cell(const Vec2u& idx, int& tile_id, Vec4c& color) const = 0;
		virtual void set_cell(const Vec2u& idx, int tile_id, const Vec4c color) = 0;

		virtual bool get_clipping() const = 0;
		virtual void set_clipping(bool c) = 0;

		FLAME_UNIVERSE_EXPORTS static cTileMap* create();
	};
}
