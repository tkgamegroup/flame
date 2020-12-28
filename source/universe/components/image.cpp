#include <flame/serialize.h>
#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "element_private.h"
#include "image_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cImagePrivate::set_res_id(int id)
	{
		if (res_id == id)
			return;
		res_id = id;
		refres_res();
		if (entity)
			entity->data_changed(this, S<"res_id"_h>);
	}

	void cImagePrivate::set_tile_id(int id)
	{
		if (tile_id == id)
			return;
		tile_id = id;
		refres_res();
		if (entity)
			entity->data_changed(this, S<"tile_id"_h>);
	}

	void cImagePrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		src = _src;
		refres_res();
		if (entity)
			entity->data_changed(this, S<"src"_h>);
	}

	void cImagePrivate::set_uv(const vec4& uv)
	{
		if (uv0 == uv.xy() && uv1 == uv.zw())
			return;
		uv0 = uv.xy;
		uv1 = uv.zw;
		if (element)
			element->mark_drawing_dirty();
		if (entity)
			entity->data_changed(this, S<"uv"_h>);
	}

	void cImagePrivate::refres_res()
	{
		iv = nullptr;
		atlas = nullptr;
		if (canvas)
		{
			if (!src.empty())
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
								if (entity)
									entity->data_changed(this, S<"res_id"_h>);
							}
							slot = tile->get_index();
							if (tile_id != slot)
							{
								tile_id = slot;
								if (entity)
									entity->data_changed(this, S<"tile_id"_h>);
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
							if (entity)
								entity->data_changed(this, S<"res_id"_h>);
						}
						if (tile_id != -1)
						{
							tile_id = -1;
							if (entity)
								entity->data_changed(this, S<"tile_id"_h>);
						}
						iv = r.iv;
					}
				}
			}
			if (!iv && res_id != -1)
			{
				if (tile_id != -1)
				{
					auto r = canvas->get_element_resource(res_id);
					atlas = r.ia;
					iv = r.ia->get_image()->get_view();
				}
				else
					iv = canvas->get_element_resource(res_id).iv;
			}
		}

		if (element)
		{
			element->mark_drawing_dirty();
			element->mark_size_dirty();
		}
	}

	void cImagePrivate::on_added()
	{
		element = entity->get_component_t<cElementPrivate>();
		fassert(element);

		drawer = element->add_drawer([](Capture& c, graphics::Canvas* canvas) {
			auto thiz = c.thiz<cImagePrivate>();
			thiz->draw(canvas);
		}, Capture().set_thiz(this));
		measurable = element->add_measurable([](Capture& c, vec2* ret) {
			auto thiz = c.thiz<cImagePrivate>();
			thiz->measure(ret);
		}, Capture().set_thiz(this));
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cImagePrivate::on_removed()
	{
		element->remove_drawer(drawer);
		element->remove_measurable(measurable);
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cImagePrivate::on_entered_world()
	{
		canvas = entity->world->get_system_t<sRendererPrivate>()->canvas;
		fassert(canvas);
		refres_res();
	}

	void cImagePrivate::on_left_world()
	{
		canvas = nullptr;
		res_id = -1;
		tile_id = -1;
		iv = nullptr;
		atlas = nullptr;
	}

	void cImagePrivate::measure(vec2* ret)
	{
		if (!iv)
		{
			*ret = vec2(-1.f);
			return;
		}
		*ret = iv->get_image()->get_size();
	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		if (res_id != -1)
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
