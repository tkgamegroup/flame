#include <flame/graphics/canvas.h>
#include <flame/universe/world.h>
#include <flame/universe/components/element.h>
#include <flame/universe/components/tile_map.h>

namespace flame
{
	struct Cell
	{
		int idx;
		Vec4c col;
	};

	struct cTileMapPrivate : cTileMap
	{
		std::vector<uint> tiles;
		std::vector<Cell> cells;

		void* draw_cmd;

		cTileMapPrivate()
		{
			element = nullptr;

			size_ = Vec2u(0);
			cell_size_ = Vec2f(0.f);

			draw_cmd = nullptr;
		}

		~cTileMapPrivate()
		{
			if (!entity->dying_)
				element->cmds.remove(draw_cmd);
		}

		void on_component_added(Component* c) override
		{
			if (c->name_hash == FLAME_CHASH("cElement"))
			{
				element = (cElement*)c;
				draw_cmd = element->cmds.add([](Capture& c, graphics::Canvas* canvas) {
					c.thiz<cTileMapPrivate>()->draw(canvas);
					return true;
				}, Capture().set_thiz(this));
			}
		}

		void draw(graphics::Canvas* canvas)
		{
			if (!element->clipped)
			{
				auto pos = element->content_min();
				auto cell_size = cell_size_ * element->global_scale;
				for (auto y = 0; y < size_.y(); y++)
				{
					for (auto x = 0; x < size_.x(); x++)
					{
						auto& c = cells[y * size_.x() + x];
						if (c.idx != -1)
							canvas->add_image(pos + Vec2f(x * cell_size.x(), y * cell_size.y()), cell_size, tiles[c.idx], Vec2f(0.f), Vec2f(1.f), c.col);
					}
				}
			}
		}
	};

	uint cTileMap::tile_count() const
	{
		return ((cTileMapPrivate*)this)->tiles.size();
	}

	uint cTileMap::tile(uint idx) const
	{
		return ((cTileMapPrivate*)this)->tiles[idx];
	}

	void cTileMap::add_tile(uint id)
	{
		auto& tiles = ((cTileMapPrivate*)this)->tiles;
		for (auto t : tiles)
		{
			if (t == id)
				return;
		}
		tiles.push_back(id);
	}

	void cTileMap::remove_tile(uint id)
	{
		auto thiz = (cTileMapPrivate*)this;
		auto& tiles = thiz->tiles;
		for (auto it = tiles.begin(); it != tiles.end(); it++)
		{
			if (*it == id)
			{
				tiles.erase(it);
				for (auto& c : thiz->cells)
				{
					if (c.idx == id)
						c.idx = -1;
				}
				return;
			}
		}
	}

	void cTileMap::set_size(const Vec2u& s)
	{
		if (size_ == s)
			return;
		auto& cells = ((cTileMapPrivate*)this)->cells;
		std::vector<Cell> new_cells(s.x() * s.y(), { -1, Vec4c(255) });
		auto mx = min(s.x(), size_.x());
		auto my = min(s.y(), size_.y());
		for (auto y = 0; y < my; y++)
		{
			for (auto x = 0; x < mx; x++)
				new_cells[y * s.x() + x] = cells[y * size_.x() + x];
		}
		size_ = s;
		cells = new_cells;
	}

	int cTileMap::cell(const Vec2i& idx) const
	{
		if (idx.x() < 0 || idx.x() >= size_.x() || idx.y() < 0 || idx.y() >= size_.y())
			return -2;
		return ((cTileMapPrivate*)this)->cells[idx.y() * size_.x() + idx.x()].idx;
	}

	void cTileMap::set_cell(const Vec2u& idx, int tile_idx, const Vec4c& col)
	{
		auto thiz = (cTileMapPrivate*)this;
		if (idx.x() >= size_.x() || idx.y() >= size_.y())
			return;
		if (tile_idx != -1 && tile_idx >= thiz->tiles.size())
			return;
		auto& c = thiz->cells[idx.y() * size_.x() + idx.x()];
		c.idx = tile_idx;
		c.col = col;
	}

	void cTileMap::clear_cells(int tile_idx, const Vec4c& col)
	{
		auto thiz = (cTileMapPrivate*)this;
		for (auto y = 0; y < size_.y(); y++)
		{
			for (auto x = 0; x < size_.x(); x++)
			{
				auto& c = thiz->cells[y * size_.x() + x];
				c.idx = tile_idx;
				c.col = col;
			}
		}
	}

	cTileMap* cTileMap::create()
	{
		return new cTileMapPrivate();
	}
}
