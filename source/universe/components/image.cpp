#include "../../graphics/device.h"
#include "../../graphics/image.h"
#include "../world_private.h"
#include "element_private.h"
#include "image_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cImagePrivate::set_src(const std::filesystem::path& _src)
	{
		if (src == _src)
			return;
		src = _src;
		apply_src();
		if (entity)
			entity->component_data_changed(this, S<"src"_h>);
	}

	void cImagePrivate::set_tile_name(const std::string& name)
	{
		if (tile_name == name)
			return;
		tile_name = name;
		apply_src();
		if (entity)
			entity->component_data_changed(this, S<"tile_name"_h>);
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
			entity->component_data_changed(this, S<"uv"_h>);
	}

	void cImagePrivate::apply_src()
	{
		res_id = -1;
		tile_id = -1;
		iv = nullptr;
		atlas = nullptr;

		if (!s_renderer || src.empty())
			return;
		auto device = graphics::Device::get_default();
		if (tile_name.empty())
		{
			auto img = graphics::Image::get(device, src.c_str(), false);
			fassert(img);
			iv = img->get_view();

			res_id = s_renderer->find_element_res(iv);
			if (res_id == -1)
				res_id = s_renderer->set_element_res(-1, iv);
		}
		else
		{
			atlas = graphics::ImageAtlas::get(device, src.c_str());
			fassert(atlas);
			iv = atlas->get_image()->get_view();

			res_id = s_renderer->find_element_res(iv);
			if (res_id == -1)
				res_id = s_renderer->set_element_res(-1, iv);

			if (!tile_name.empty())
			{
				graphics::ImageAtlas::TileInfo ti;
				tile_id = atlas->find_tile(tile_name.c_str(), &ti);
				fassert(tile_id != -1);
				tile_uv = ti.uv;
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
		fassert(element);

		drawer = element->add_drawer([](Capture& c, uint layer, sRendererPtr s_renderer) {
			auto thiz = c.thiz<cImagePrivate>();
			return thiz->draw(layer, s_renderer);
		}, Capture().set_thiz(this));
		measurable = element->add_measurer([](Capture& c, vec2* s) {
			auto thiz = c.thiz<cImagePrivate>();
			return thiz->measure(s);
		}, Capture().set_thiz(this));
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cImagePrivate::on_removed()
	{
		element->remove_drawer(drawer);
		element->remove_measurer(measurable);
		element->mark_drawing_dirty();
		element->mark_size_dirty();
	}

	void cImagePrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);
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
		*s = iv->get_image()->get_size();
		return true;
	}

	uint cImagePrivate::draw(uint layer, sRenderer* s_renderer)
	{
		if (res_id != -1)
		{
			layer++;
			if (tile_id == -1)
				s_renderer->draw_image(layer, element->points + 4, res_id, uv0, uv1, cvec4(255));
			else
			{
				auto _uv0 = mix(tile_uv.xy(), tile_uv.zw(), uv0);
				auto _uv1 = mix(tile_uv.xy(), tile_uv.zw(), uv1);
				s_renderer->draw_image(layer, element->points + 4, res_id, _uv0, _uv1, cvec4(255));
			}
		}
		return layer;
	}

	cImage* cImage::create(void* parms)
	{
		return new cImagePrivate();
	}
}
