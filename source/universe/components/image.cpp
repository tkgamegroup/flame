#include <flame/graphics/image.h>
#include <flame/graphics/canvas.h>
#include "../world_private.h"
#include "../res_map_private.h"
#include "image_private.h"
#include "aligner_private.h"

namespace flame
{
	void cImagePrivate::set_res_id(uint id)
	{
		if (res_id == id)
			return;
		res_id = id;
		Entity::report_data_changed(this, S<ch("res_id")>::v);
		if (!src.empty())
		{
			src = "";
			Entity::report_data_changed(this, S<ch("src")>::v);
		}
	}

	void cImagePrivate::set_tile_id(uint id)
	{
		if (tile_id == id)
			return;
		tile_id = id;
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
	}

	void cImagePrivate::on_gain_element()
	{
		element->drawers.push_back(this);
	}

	void cImagePrivate::on_lost_element()
	{
		std::erase_if(element->drawers, [&](const auto& i) {
			return i == this;
		});
	}

	Vec2f cImagePrivate::measure()
	{
		if (!canvas || res_id == 0xffffffff)
			return Vec2f(0.f);
		auto r = canvas->get_resource(res_id);
		auto ia = r->get_image_atlas();
		if (!ia)
			return Vec2f(r->get_view()->get_image()->get_size());
		return Vec2f(ia->get_tile(tile_id)->get_size());
	}

	void cImagePrivate::draw(graphics::Canvas* canvas)
	{
		canvas->add_image(res_id, tile_id, 
			element->points[4], element->points[5], element->points[6], element->points[7],
			uv0, uv1, Vec4c(255));
	}

	void cImagePrivate::on_added()
	{
		on_entity_message(MessageElementSizeDirty);
	}

	void cImagePrivate::on_entity_message(Message msg)
	{
		switch (msg)
		{
		case MessageElementSizeDirty:
			if (type_setting && true/*auto_size*/)
				type_setting->add_to_sizing_list(this, (EntityPrivate*)entity);
			break;
		}
	}

	void cImagePrivate::on_gain_type_setting()
	{
		on_entity_message(MessageElementSizeDirty);
	}

	void cImagePrivate::on_lost_type_setting()
	{
		type_setting->remove_from_sizing_list(this);
	}

	void cImagePrivate::on_gain_canvas()
	{
		apply_src();
	}

	void cImagePrivate::on_gain_res_map()
	{
		apply_src();
	}

	void cImagePrivate::apply_src()
	{
		res_id = 0xffffffff;
		tile_id = 0;
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
