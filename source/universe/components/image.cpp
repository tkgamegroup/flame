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
		data_changed(S<"res_id"_h>);
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
		data_changed(S<"tile_id"_h>);
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
		data_changed(S<"src"_h>);
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
							data_changed(S<"res_id"_h>);
						}
						slot = tile->get_index();
						if (tile_id != slot)
						{
							tile_id = slot;
							data_changed(S<"tile_id"_h>);
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
						data_changed(S<"res_id"_h>);
					}
					if (tile_id != -1)
					{
						tile_id = -1;
						data_changed(S<"tile_id"_h>);
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

	void cImagePrivate::measure(vec2& ret)
	{
		if (!iv)
		{
			ret = vec2(-1.f);
			return;
		}
		ret = iv->get_image()->get_size();
	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		if (res_id != -1 && tile_id != -1)
		{
			canvas->draw_image(res_id, tile_id, element->points[4], 
				element->content_size, element->axes, uv0, uv1, cvec4(255));
		}
	}

	cImage* cImage::create()
	{
		return f_new<cImagePrivate>();
	}
}
