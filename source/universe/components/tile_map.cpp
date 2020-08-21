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
		Entity::report_data_changed(this, S<ch("res_id")>::v);
	}

	void cTileMapPrivate::set_cell_count(const Vec2u& c)
	{
		if (cell_count == c)
			return;
		cell_count = c;

		cells.resize(c.y());
		for (auto& v : cells)
			v.resize(c.x());

		if (element)
			element->mark_drawing_dirty();
		Entity::report_data_changed(this, S<ch("cell_count")>::v);
	}

	void cTileMapPrivate::set_cell_size(const Vec2f& s)
	{
		if (cell_size == s)
			return;
		cell_size = s;
		if (element)
			element->mark_drawing_dirty();
		Entity::report_data_changed(this, S<ch("cell_size")>::v);
	}

	void cTileMapPrivate::get_cell(const Vec2u& idx, int& tile_id, Vec4c& color) const
	{
		if (idx.x() >= cell_count.x() || idx.y() >= cell_count.y())
			return;
		auto& cell = cells[idx.y()][idx.x()];
		tile_id = cell.first;
		color = cell.second;
	}

	void cTileMapPrivate::set_cell(const Vec2u& idx, int tile_id, const Vec4c color)
	{
		if (idx.x() >= cell_count.x() || idx.y() >= cell_count.y())
			return;
		auto& cell = cells[idx.y()][idx.x()];
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
		Entity::report_data_changed(this, S<ch("clipping")>::v);
	}

	void cTileMapPrivate::on_gain_element()
	{
		element->after_drawers.push_back(this);
		element->mark_drawing_dirty();
	}

	void cTileMapPrivate::on_lost_element()
	{
		std::erase_if(element->after_drawers, [&](const auto& i) {
			return i == this;
		});
		element->mark_drawing_dirty();
	}

	void cTileMapPrivate::draw(graphics::Canvas* canvas)
	{
		if (res_id == -1 || cell_count == Vec2u(0) || cell_size == Vec2f(0.f))
			return;

		auto axes = element->axes;
		auto p = element->points[4];
		auto l = 0;
		auto r = (int)cell_count.x();
		auto t = 0;
		auto b = (int)cell_count.y();

		if (clipping)
		{
			auto inv = inverse(axes);
			auto wtf = inv * axes;
			Vec2f ps[4];
			ps[0] = inv * Vec2f(element->scissor[0] - p.x(), element->scissor[1] - p.y());
			ps[1] = inv * Vec2f(element->scissor[2] - p.x(), element->scissor[1] - p.y());
			ps[2] = inv * Vec2f(element->scissor[2] - p.x(), element->scissor[3] - p.y());
			ps[3] = inv * Vec2f(element->scissor[0] - p.x(), element->scissor[3] - p.y());
			Vec4f bb;
			bb[0] = bb[2] = ps[0].x();
			bb[1] = bb[3] = ps[0].y();
			for (auto i = 1; i < 4; i++)
			{
				bb[0] = min(bb[0], ps[i].x());
				bb[2] = max(bb[2], ps[i].x());
				bb[1] = min(bb[1], ps[i].y());
				bb[3] = max(bb[3], ps[i].y());
			}
			l = max((int)(bb[0] / cell_size.x()), 0);
			r = min((int)(bb[2] / cell_size.x()) + 1, r);
			t = max((int)(bb[1] / cell_size.y()), 0);
			b = min((int)(bb[3] / cell_size.y()) + 1, b);
		}

		for (auto i = t; i < b; i++)
		{
			for (auto j = l; j < r; j++)
			{
				auto& cell = cells[i][j];
				if (cell.first != -1)
				{
					canvas->add_image(res_id, cell.first,
						p + axes * (Vec2f(j, i) * cell_size),
						p + axes * (Vec2f(j + 1, i) * cell_size),
						p + axes * (Vec2f(j + 1, i + 1) * cell_size),
						p + axes * (Vec2f(j, i + 1) * cell_size),
						Vec2f(0.f), Vec2f(1.f), cell.second);
				}
			}
		}
	}

	cTileMap* cTileMap::create()
	{
		return f_new<cTileMapPrivate>();
	}
}
