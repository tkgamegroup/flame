#include <flame/graphics/canvas.h>
#include "../entity_private.h"
#include "tile_map_private.h"

namespace flame
{
	void cTileMapPrivate::set_res_id(int id)
	{
		if (res_id == id)
			return;
		res_id = id;
		if (element)
			element->mark_drawing_dirty();
		Entity::report_data_changed(this, S<"res_id"_h>);
	}

	void cTileMapPrivate::set_cell_count(const uvec2& c)
	{
		if (cell_count == c)
			return;
		cell_count = c;

		cells.resize(c.y);
		for (auto& v : cells)
			v.resize(c.x);

		if (element)
			element->mark_drawing_dirty();
		Entity::report_data_changed(this, S<"cell_count"_h>);
	}

	void cTileMapPrivate::set_cell_size(const vec2& s)
	{
		if (cell_size == s)
			return;
		cell_size = s;
		if (element)
			element->mark_drawing_dirty();
		Entity::report_data_changed(this, S<"cell_size"_h>);
	}

	void cTileMapPrivate::get_cell(const uvec2& idx, int& tile_id, cvec4& color) const
	{
		if (idx.x >= cell_count.x || idx.y >= cell_count.y)
			return;
		auto& cell = cells[idx.y][idx.x];
		tile_id = cell.first;
		color = cell.second;
	}

	void cTileMapPrivate::set_cell(const uvec2& idx, int tile_id, const cvec4 color)
	{
		if (idx.x >= cell_count.x || idx.y >= cell_count.y)
			return;
		auto& cell = cells[idx.y][idx.x];
		cell.first = tile_id;
		cell.second = color;
		if (element)
			element->mark_drawing_dirty();
	}

	void cTileMapPrivate::set_clipping(bool c)
	{
		if (clipping == c)
			return;
		clipping = c;
		if (element)
			element->mark_drawing_dirty();
		Entity::report_data_changed(this, S<"clipping"_h>);
	}

	void cTileMapPrivate::draw(graphics::Canvas* canvas)
	{
		if (res_id == -1 || cell_count == uvec2(0) || cell_size == vec2(0.f))
			return;

		auto axes = element->axes;
		auto p = element->points[4];
		auto l = 0;
		auto r = (int)cell_count.x;
		auto t = 0;
		auto b = (int)cell_count.y;

		if (clipping)
		{
			auto inv = inverse(axes);
			vec2 ps[4];
			ps[0] = inv * vec2(element->parent_scissor.LT.x - p.x, element->parent_scissor.LT.y - p.y);
			ps[1] = inv * vec2(element->parent_scissor.RB.x - p.x, element->parent_scissor.LT.y - p.y);
			ps[2] = inv * vec2(element->parent_scissor.RB.x - p.x, element->parent_scissor.RB.y - p.y);
			ps[3] = inv * vec2(element->parent_scissor.LT.x - p.x, element->parent_scissor.RB.y - p.y);
			vec4 bb;
			bb[0] = bb[2] = ps[0].x;
			bb[1] = bb[3] = ps[0].y;
			for (auto i = 1; i < 4; i++)
			{
				bb[0] = min(bb[0], ps[i].x);
				bb[2] = max(bb[2], ps[i].x);
				bb[1] = min(bb[1], ps[i].y);
				bb[3] = max(bb[3], ps[i].y);
			}
			l = max((int)(bb[0] / cell_size.x), 0);
			r = min((int)(bb[2] / cell_size.x) + 1, r);
			t = max((int)(bb[1] / cell_size.y), 0);
			b = min((int)(bb[3] / cell_size.y) + 1, b);
		}

		for (auto i = t; i < b; i++)
		{
			for (auto j = l; j < r; j++)
			{
				auto& cell = cells[i][j];
				if (cell.first != -1)
				{
					canvas->draw_image(res_id, cell.first,
						p + axes * (vec2(j, i) * cell_size),
						cell_size, element->axes,
						vec2(0.f), vec2(1.f), cell.second);
				}
			}
		}
	}

	cTileMap* cTileMap::create()
	{
		return f_new<cTileMapPrivate>();
	}
}
