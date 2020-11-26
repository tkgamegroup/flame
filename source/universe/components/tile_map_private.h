#pragma once

#include "element_private.h"
#include <flame/universe/components/tile_map.h>

namespace flame
{
	struct cElementPrivate;

	struct cTileMapPrivate : cTileMap // R ~ on_*
	{
		int res_id = -1;
		uvec2 cell_count = uvec2(0);
		vec2 cell_size = vec2(0.f);

		std::vector<std::vector<std::pair<int, cvec4>>> cells;

		bool clipping = false;

		cElementPrivate* element = nullptr; // R ref

		int get_res_id() const override { return res_id; }
		void set_res_id(int id) override;

		uvec2 get_cell_count() const override { return cell_count; }
		void set_cell_count(const uvec2& c) override;
		vec2 get_cell_size() const override { return cell_size; }
		void set_cell_size(const vec2& s) override;

		void get_cell(const uvec2& idx, int& tile_id, cvec4& color) const override;
		void set_cell(const uvec2& idx, int tile_id, const cvec4 color) override;

		bool get_clipping() const override { return clipping; }
		void set_clipping(bool c) override;

		void draw(graphics::Canvas* canvas); // R
	};
}
