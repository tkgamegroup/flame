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
		Entity::report_data_changed(this, S<ch("src")>::v);
	}

	void cImagePrivate::apply_src()
	{
		iv = nullptr;
		atlas = nullptr;
		if (canvas && !src.empty())
		{
			auto sp = SUS::split(src, '.');
			auto slot = canvas->find_element_resource(sp[0].c_str());
			auto r = canvas->get_element_resource(slot);
			if (r.ia)
			{
				if (sp.size() == 2)
				{
					auto tile = r.ia->find_tile(sp[1].c_str());
					if (tile)
					{
						if (res_id != slot)
						{
							res_id = slot;
							Entity::report_data_changed(this, S<ch("res_id")>::v);
						}
						slot = tile->get_index();
						if (tile_id != slot)
						{
							tile_id = slot;
							Entity::report_data_changed(this, S<ch("tile_id")>::v);
						}
						iv = r.ia->get_image()->get_view();
						atlas = r.ia;
					}
				}
			}
			else
			{
				if (sp.size() == 1)
				{
					if (res_id != slot)
					{
						res_id = slot;
						Entity::report_data_changed(this, S<ch("res_id")>::v);
					}
					if (tile_id != -1)
					{
						tile_id = -1;
						Entity::report_data_changed(this, S<ch("tile_id")>::v);
					}
					iv = r.iv;
				}
			}
		}
	}

	void cImagePrivate::on_gain_canvas()
	{
		apply_src();
	}

	void cImagePrivate::on_lost_canvas()
	{
		res_id = -1;
		tile_id = -1;
		iv = nullptr;
		atlas = nullptr;
	}

	void cImagePrivate::measure(Vec2f& ret)
	{
		if (!iv)
		{
			ret = Vec2f(-1.f);
			return;
		}
		ret = iv->get_image()->get_size();
	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		if (res_id != -1 && tile_id != -1)
		{
			canvas->draw_image(res_id, tile_id,
				element->global_points[4], element->global_points[5], element->global_points[6], element->global_points[7],
				uv0, uv1, Vec4c(255));
		}
	}

	cImage* cImage::create()
	{
		return f_new<cImagePrivate>();
	}
}
