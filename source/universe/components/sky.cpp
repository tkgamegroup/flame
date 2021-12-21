#include "../../graphics/device.h"
#include "../../graphics/image.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "sky_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cSkyPrivate::set_box_texture_path(const std::filesystem::path& path)
	{
		if (box_texture_path == path)
			return;
		box_texture_path = path;
	}

	void cSkyPrivate::set_irr_texture_path(const std::filesystem::path& path)
	{
		if (irr_texture_path == path)
			return;
		irr_texture_path = path;
	}

	void cSkyPrivate::set_rad_texture_path(const std::filesystem::path& path)
	{
		if (rad_texture_path == path)
			return;
		rad_texture_path = path;
	}

	void cSkyPrivate::set_lut_texture_path(const std::filesystem::path& path)
	{
		if (lut_texture_path == path)
			return;
		lut_texture_path = path;
	}

	void cSkyPrivate::set_intensity(float i)
	{
		if (intensity == i)
			return;
		intensity = i;
		if (s_renderer)
		{
			if (s_renderer->get_sky_id() == this)
				s_renderer->set_sky(box_texture_view, irr_texture_view, rad_texture_view, lut_texture_view, fog_color, intensity, this);
			s_renderer->mark_dirty();
		}
		data_changed(S<"intensity"_h>);
	}

	void cSkyPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

		auto ppath = entity->get_src(src_id).parent_path();

		{
			auto fn = std::filesystem::path(box_texture_path);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			box_texture = graphics::Image::get(nullptr, fn.c_str(), true);
			if (box_texture)
			{
				auto lv = (uint)box_texture->get_levels();
				box_texture_view = box_texture->get_view({ 0, lv, 0, 6 });
				lv--;
				vec4 color = vec4(0.f);
				for (auto i = 0; i < 6; i++)
					color += box_texture->linear_sample(vec2(0.5f), lv, i);
				color /= 6.f;
				fog_color = color;
			}
		}
		{
			auto fn = std::filesystem::path(irr_texture_path);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			irr_texture = graphics::Image::get(nullptr, fn.c_str(), true);
			if (irr_texture)
				irr_texture_view = irr_texture->get_view({ 0, irr_texture->get_levels(), 0, 6 });
		}
		{
			auto fn = std::filesystem::path(rad_texture_path);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			rad_texture = graphics::Image::get(nullptr, fn.c_str(), true);
			if (rad_texture)
				rad_texture_view = rad_texture->get_view({ 0, rad_texture->get_levels(), 0, 6 });
		}
		{
			auto fn = std::filesystem::path(lut_texture_path);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			lut_texture = graphics::Image::get(nullptr, fn.c_str(), true);
			if (lut_texture)
				lut_texture_view = lut_texture->get_view();
		}

		s_renderer->set_sky(box_texture_view, irr_texture_view, rad_texture_view, lut_texture_view, fog_color, intensity, this);
	}

	void cSkyPrivate::on_left_world()
	{
		box_texture = nullptr;
		box_texture_view = nullptr;
		irr_texture = nullptr;
		irr_texture_view = nullptr;
		rad_texture = nullptr;
		rad_texture_view = nullptr;
		lut_texture = nullptr;
		lut_texture_view = nullptr;

		if (s_renderer->get_sky_id() == this)
			s_renderer->set_sky(nullptr, nullptr, nullptr, nullptr, vec3(1.f), 1.f, nullptr);

		s_renderer = nullptr;
	}

	cSky* cSky::create()
	{
		return new cSkyPrivate();
	}
}
