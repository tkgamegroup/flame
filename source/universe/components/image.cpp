#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "element_private.h"
#include "image_private.h"

namespace flame
{
	void cImagePrivate::set_res_id(int id)
	{
		if (res_id == id)
			return;
		res_id = id;
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		Entity::report_data_changed(this, S<ch("res_id")>::v);
		if (!src.empty())
		{
			src = "";
			Entity::report_data_changed(this, S<ch("src")>::v);
		}
	}

	void cImagePrivate::set_tile_id(int id)
	{
		if (tile_id == id)
			return;
		tile_id = id;
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
		Entity::report_data_changed(this, S<ch("tile_id")>::v);
		if (!src.empty())
		{
			src = "";
			Entity::report_data_changed(this, S<ch("src")>::v);
		}
	}

	void cImagePrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
	}

	void cImagePrivate::measure(Vec2f& ret)
	{
		if (!canvas || res_id == -1 || tile_id == -1 || !auto_size)
		{
			ret = Vec2f(-1.f);
			return;
		}
		auto r = canvas->get_resource(res_id);
		auto ia = r->get_image_atlas();
		if (!ia)
			ret = Vec2f(r->get_view()->get_image()->get_size());
		else
			ret = Vec2f(ia->get_tile(tile_id)->get_size());

	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		if (res_id != -1 && tile_id != -1)
		{
			canvas->draw_image(res_id, tile_id,
				element->points[4], element->points[5], element->points[6], element->points[7],
				uv0, uv1, Vec4c(255));
		}
	}

	void cImagePrivate::on_gain_canvas()
	{
		apply_src();
	}

	void cImagePrivate::apply_src()
	{
		res_id = -1;
		tile_id = -1;
		if (canvas && !src.empty())
		{
			auto sp = SUS::split(src, '.');
			auto slot = 0;
			while (true)
			{
				auto r = canvas->get_resource(slot);
				if (!r)
					break;
				if (r->get_name() == sp[0])
				{
					auto atlas = r->get_image_atlas();
					if (!atlas)
					{
						assert(sp.size() == 1);
						res_id = slot;
					}
					else
					{
						assert(sp.size() == 2);
						auto tile = atlas->find_tile(sp[1].c_str());
						if (tile)
						{
							res_id = slot;
							tile_id = tile->get_index();
						}
					}
					Entity::report_data_changed(this, S<ch("res_id")>::v);
					Entity::report_data_changed(this, S<ch("tile_id")>::v);
					Entity::report_data_changed(this, S<ch("src")>::v);
					break;
				}
				slot++;
			}
		}
	}

	cImage* cImage::create()
	{
		return f_new<cImagePrivate>();
	}
}
