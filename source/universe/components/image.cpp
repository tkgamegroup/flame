#include <flame/graphics/canvas.h>

#include "../world_private.h"
#include "../res_map_private.h"
#include "image_private.h"

namespace flame
{
	void cImagePrivate::set_res_id(uint id)
	{
		if (res_id == id)
			return;
		res_id = id;
		src = "";
	}

	void cImagePrivate::set_tile_id(uint id)
	{
		if (tile_id == id)
			return;
		tile_id = id;
		src = "";
	}

	void cImagePrivate::set_src(const std::string& _src)
	{
		if (src == _src)
			return;
		res_id = 0xffffffff;
		tile_id = 0;
		src = _src;
		if (canvas && res_map)
		{
			auto path = res_map->get_res_path(src);
			auto slot = 0;
			while (true)
			{
				auto r = canvas->get_resource(slot);
				if (!r)
					break;
				auto r_filename = std::filesystem::path(r->get_filename());
				if (r_filename == path)
				{
					res_id = slot;
					break;
				}
				slot++;
			}
		}
	}

	void cImagePrivate::on_added()
	{
		element = (cElementPrivate*)((EntityPrivate*)entity)->get_component(cElement::type_hash);
		element->drawers.push_back(this);
	}

	void cImagePrivate::on_removed()
	{
		std::erase_if(element->drawers, [&](const auto& i) {
			return i == this;
		});
	}

	void cImagePrivate::on_entered_world()
	{
		auto world = ((EntityPrivate*)entity)->world;
		canvas = (graphics::Canvas*)world->find_object("Canvas");
		res_map = (ResMapPrivate*)world->find_object("ResMap");
		if (canvas && res_map && !src.empty())
		{
			auto _src = src;
			src = "";
			set_src(_src);
		}
	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		//if (!element->clipped)
		{
			auto p1 = element->get_p00();
			auto p2 = element->get_p11() - p1;
			canvas->add_image(res_id, tile_id, p1, p2, uv0, uv1, Vec4c(255));
		}
	}

	cImagePrivate* cImagePrivate::create()
	{
		auto ret = _allocate(sizeof(cImagePrivate));
		new (ret) cImagePrivate;
		return (cImagePrivate*)ret;
	}

	cImage* cImage::create() { return cImagePrivate::create(); }
}
