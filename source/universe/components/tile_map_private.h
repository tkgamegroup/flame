#pragma once

#include "element_private.h"
#include <flame/universe/components/tile_map.h>

namespace flame
{
	struct cElementPrivate;

	struct cTileMapPrivate : cTileMap, cElement::Drawer // R ~ on_*
	{
		int res_id = -1;
		Vec2u cell_count = Vec2u(0);
		Vec2f cell_size = Vec2f(0.f);

		std::vector<std::vector<std::pair<int, Vec4c>>> cells;

		cElementPrivate* element = nullptr; // R ref

		int get_res_id() const override { return res_id; }
		void set_res_id(int id) override;

		Vec2u get_cell_count() const override { return cell_count; }
		void set_cell_count(const Vec2u& c) override;
		Vec2f get_cell_size() const override { return cell_size; }
		void set_cell_size(const Vec2f& s) override;

		void get_cell(const Vec2u& idx, int& tile_id, Vec4c& color) const override;
		void set_cell(const Vec2u& idx, int tile_id, const Vec4c color) override;

		void on_gain_element();
		void on_lost_element();

		void draw(graphics::Canvas* canvas) override;
	};
}
