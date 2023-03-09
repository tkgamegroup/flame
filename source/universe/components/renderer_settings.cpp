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
			AssetManagemant::release(Path::get(sky_map_name));
		sky_map_name = name;
		if (!sky_map_name.empty())
			AssetManagemant::get(Path::get(sky_map_name));

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

	void cRendererSettingsPrivate::set_post_processing_enable(bool v)
	{
		if (post_processing_enable == v)
			return;
		post_processing_enable = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_post_processing_enable(post_processing_enable);
	}

	void cRendererSettingsPrivate::set_ssao_enable(bool v)
	{
		if (ssao_enable == v)
			return;
		ssao_enable = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssao_enable(ssao_enable);
	}

	void cRendererSettingsPrivate::set_ssao_radius(float v)
	{
		if (ssao_radius == v)
			return;
		ssao_radius = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssao_radius(ssao_radius);
	}

	void cRendererSettingsPrivate::set_ssao_bias(float v)
	{
		if (ssao_bias == v)
			return;
		ssao_bias = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssao_bias(ssao_bias);
	}

	void cRendererSettingsPrivate::set_white_point(float v)
	{
		if (white_point == v)
			return;
		white_point = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_white_point(white_point);
	}

	void cRendererSettingsPrivate::set_bloom_enable(bool v)
	{
		if (bloom_enable == v)
			return;
		bloom_enable = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_bloom_enable(bloom_enable);
	}

	void cRendererSettingsPrivate::set_ssr_enable(bool v)
	{
		if (ssr_enable == v)
			return;
		ssr_enable = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssr_enable(ssr_enable);
	}

	void cRendererSettingsPrivate::set_ssr_thickness(float v)
	{
		if (ssr_thickness == v)
			return;
		ssr_thickness = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssr_thickness(ssr_thickness);
	}

	void cRendererSettingsPrivate::set_ssr_max_distance(float v)
	{
		if (ssr_max_distance == v)
			return;
		ssr_max_distance = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssr_max_distance(ssr_max_distance);
	}

	void cRendererSettingsPrivate::set_ssr_max_steps(uint v)
	{
		if (ssr_max_steps == v)
			return;
		ssr_max_steps = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssr_max_steps(ssr_max_steps);
	}

	void cRendererSettingsPrivate::set_ssr_binary_search_steps(uint v)
	{
		if (ssr_binary_search_steps == v)
			return;
		ssr_binary_search_steps = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_ssr_binary_search_steps(ssr_binary_search_steps);
	}

	void cRendererSettingsPrivate::set_tone_mapping_enable(bool v)
	{
		if (tone_mapping_enable == v)
			return;
		tone_mapping_enable = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_tone_mapping_enable(tone_mapping_enable);
	}

	void cRendererSettingsPrivate::set_gamma(float v)
	{
		if (gamma == v)
			return;
		gamma = v;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_gamma(gamma);
	}

	void cRendererSettingsPrivate::set_post_shading_code_file(const std::filesystem::path& path)
	{
		if (post_shading_code_file == path)
			return;
		post_shading_code_file = path;

		if (!settings.empty() && settings.front() == this)
			sRenderer::instance()->set_post_shading_code_file(post_shading_code_file);
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
			sRenderer::instance()->set_shadow_distance(shadow_distance);
			sRenderer::instance()->set_csm_levels(csm_levels);
			sRenderer::instance()->set_esm_factor(esm_factor);
			sRenderer::instance()->set_post_processing_enable(post_processing_enable);
			sRenderer::instance()->set_ssr_enable(ssr_enable);
			sRenderer::instance()->set_ssr_thickness(ssr_thickness);
			sRenderer::instance()->set_ssr_max_distance(ssr_max_distance);
			sRenderer::instance()->set_ssr_max_steps(ssr_max_steps);
			sRenderer::instance()->set_ssr_binary_search_steps(ssr_binary_search_steps);
			sRenderer::instance()->set_tone_mapping_enable(tone_mapping_enable);
			sRenderer::instance()->set_post_shading_code_file(post_shading_code_file);
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
