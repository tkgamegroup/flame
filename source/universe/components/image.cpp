#include "../../graphics/device.h"
#include "../../graphics/image.h"
#include "../world_private.h"
#include "image_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cImagePrivate::set_img(const std::filesystem::path& src)
	{
		if (img_src == src)
			return;
		img_src = src;
		apply_src();
		data_changed(S<"img"_h>);
	}

	void cImagePrivate::set_tile(std::string_view name)
	{
		if (tile_name == name)
			return;
		tile_name = name;
		apply_src();
		data_changed(S<"tile"_h>);
	}

	void cImagePrivate::set_uv(const vec4& _uvs)
	{
		if (uvs == _uvs)
			return;
		uvs = _uvs;
		if (element)
			element->mark_drawing_dirty();
		data_changed(S<"uv"_h>);
	}

	void cImagePrivate::apply_src()
	{
		res_id = -1;
		tile_id = -1;
		iv = nullptr;
		atlas = nullptr;

		if (!s_renderer || img_src.empty())
			return;

		if (img_src.extension() != L".atlas")
		{
			auto img = graphics::Image::get(nullptr, img_src.c_str(), false);
			assert(img);
			iv = img->get_view();
			tile_sz = img->get_size();

			res_id = s_renderer->find_element_res(iv);
			if (res_id == -1)
				res_id = s_renderer->set_element_res(-1, iv, nullptr);
		}
		else
		{
			atlas = graphics::ImageAtlas::get(nullptr, img_src.c_str());
			assert(atlas);
			iv = atlas->get_image()->get_view();

			res_id = s_renderer->find_element_res(iv);
			if (res_id == -1)
				res_id = s_renderer->set_element_res(-1, iv, nullptr);

			if (!tile_name.empty())
			{
				graphics::ImageAtlas::TileInfo ti;
				tile_id = atlas->find_tile(tile_name.c_str(), &ti);
				assert(tile_id != -1);
				tile_uv = ti.uv;
				tile_sz = ti.size;
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
		element = entity->get_component_i<cElementPrivate>(0);
		assert(element);

		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cImagePrivate::on_removed()
	{
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cImagePrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);
		apply_src();
	}

	void cImagePrivate::on_left_world()
	{
		s_renderer = nullptr;
		res_id = -1;
		tile_id = -1;
		iv = nullptr;
		atlas = nullptr;
	}

	bool cImagePrivate::measure(vec2* s)
	{
		if (!iv)
			return false;
		if (iv)
			*s = tile_sz;
		return true;
	}

	uint cImagePrivate::draw(uint layer, sRendererPtr s_renderer)
	{
		if (res_id != -1)
		{
			layer++;
			if (!atlas)
				s_renderer->draw_image(layer, element->points + 4, res_id, uvs, cvec4(255));
			else if(tile_id != -1)
			{
				auto _uvs = vec4(mix(tile_uv.xy(), tile_uv.zw(), uvs.xy()),
					mix(tile_uv.xy(), tile_uv.zw(), uvs.zw()));
				s_renderer->draw_image(layer, element->points + 4, res_id, _uvs, cvec4(255));
			}
		}
		return layer;
	}

	cImage* cImage::create()
	{
		return new cImagePrivate();
	}
}
