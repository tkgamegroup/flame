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

	void cTileMapPrivate::on_gain_element()
	{
		element->drawers.push_back(this);
		element->mark_drawing_dirty();
	}

	void cTileMapPrivate::on_lost_element()
	{
		std::erase_if(element->drawers, [&](const auto& i) {
			return i == this;
		});
		element->mark_drawing_dirty();
	}

	void cTileMapPrivate::draw(graphics::Canvas* canvas)
	{
		if (res_id == -1 || cell_count == Vec2u(0) || cell_size == Vec2f(0.f))
			return;
		auto p = element->points[4];
		auto axes = Mat2f(element->transform);
		for (auto i = 0; i < cell_count.y(); i++)
		{
			for (auto j = 0; j < cell_count.x(); j++)
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
