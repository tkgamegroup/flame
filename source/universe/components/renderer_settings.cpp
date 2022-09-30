#include "../../graphics/image.h"
#include "../entity_private.h"
#include "renderer_settings_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	std::vector<cRendererSettingsPtr> settings;

	void cRendererSettingsPrivate::set_sky_map_name(const std::filesystem::path& name)
	{
		if (sky_map_name == name)
			return;
		if (!sky_map_name.empty())
			AssetManagemant::release_asset(Path::get(sky_map_name));
		sky_map_name = name;
		if (!sky_map_name.empty())
			AssetManagemant::get_asset(Path::get(sky_map_name));

		auto _sky_map = !sky_map_name.empty() ? graphics::Image::get(sky_map_name) : nullptr;
		if (sky_map != _sky_map)
		{
			if (sky_map)
				graphics::Image::release(sky_map);
			sky_map = _sky_map;
		}
		else if (_sky_map)
			graphics::Image::release(_sky_map);

		auto _sky_irr_map = !sky_map_name.empty() ? graphics::Image::get(replace_fn(sky_map_name, L"{}_irr")) : nullptr;
		if (sky_irr_map != _sky_irr_map)
		{
			if (sky_irr_map)
				graphics::Image::release(sky_irr_map);
			sky_irr_map = _sky_irr_map;
		}
		else if (_sky_irr_map)
			graphics::Image::release(_sky_irr_map);

		auto _sky_rad_map = !sky_map_name.empty() ? graphics::Image::get(replace_fn(sky_map_name, L"{}_rad")) : nullptr;
		if (sky_rad_map != _sky_rad_map)
		{
			if (sky_rad_map)
				graphics::Image::release(sky_rad_map);
			sky_rad_map = _sky_rad_map;
		}
		else if (_sky_rad_map)
			graphics::Image::release(_sky_rad_map);

		if (!settings.empty() && settings.front() == this)
			update_sky();

		data_changed("sky_map_name"_h);
	}

	void cRendererSettingsPrivate::set_sky_intensity(float v)
	{
		if (sky_intensity == v)
			return;
		sky_intensity = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_sky_intensity(sky_intensity);
	}

	void cRendererSettingsPrivate::set_fog_color(const vec3& color)
	{
		if (fog_color == color)
			return;
		fog_color = color;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_fog_color(fog_color);
	}

	void cRendererSettingsPrivate::set_shadow_distance(float d)
	{
		if (shadow_distance == d)
			return;
		shadow_distance = d;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_shadow_distance(shadow_distance);
	}

	void cRendererSettingsPrivate::set_csm_levels(uint lv)
	{
		if (csm_levels == lv)
			return;
		csm_levels = lv;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_csm_levels(csm_levels);
	}

	void cRendererSettingsPrivate::set_esm_factor(float f)
	{
		if (esm_factor == f)
			return;
		esm_factor = f;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_esm_factor(esm_factor);
	}

	void cRendererSettingsPrivate::update_sky()
	{
		sRenderer::instance()->set_sky_maps(sky_map ? sky_map->get_view({ 0, 1, 0, 6 }) : nullptr,
			sky_irr_map ? sky_irr_map->get_view({ 0, 1, 0, 6 }) : nullptr,
			sky_rad_map ? sky_rad_map->get_view({ 0, sky_rad_map->n_levels, 0, 6 }) : nullptr);
	}

	void cRendererSettingsPrivate::on_active()
	{
		if (settings.empty())
			settings.push_back(this);
		else
		{
			for (auto it = settings.begin(); it != settings.end(); it++)
			{
				if (entity->compare_depth((*it)->entity))
				{
					settings.insert(it, this);
					return;
				}
			}
		}

		if (!settings.empty() && settings.front() == this)
		{
			update_sky();
			sRenderer::instance()->set_sky_intensity(sky_intensity);
			sRenderer::instance()->set_fog_color(fog_color);
		}
	}

	void cRendererSettingsPrivate::on_inactive()
	{
		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_sky_maps(nullptr, nullptr, nullptr);
		std::erase_if(settings, [&](auto c) {
			return c == this;
		});
	}

	struct cRendererSettingsCreate : cRendererSettings::Create
	{
		cRendererSettingsPtr operator()(EntityPtr e) override
		{
			return new cRendererSettingsPrivate();
		}
	}cRendererSettings_create;
	cRendererSettings::Create& cRendererSettings::create = cRendererSettings_create;
}
