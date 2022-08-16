#include "../../graphics/image.h"
#include "../entity_private.h"
#include "environment_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	std::vector<cEnvironmentPtr> environments;

	void cEnvironmentPrivate::set_sky_map_name(const std::filesystem::path& name)
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

		if (!environments.empty() && environments.front() == this)
			update_sky();

		data_changed("sky_map_name"_h);
	}

	void cEnvironmentPrivate::set_sky_intensity(float v)
	{
		if (sky_intensity == v)
			return;
		sky_intensity = v;

		if (!environments.empty() && environments.front() == this)
			sRenderer::instance()->set_sky_intensity(sky_intensity);
	}

	void cEnvironmentPrivate::set_fog_color(const vec3& color)
	{
		if (fog_color == color)
			return;
		fog_color = color;

		if (!environments.empty() && environments.front() == this)
			sRenderer::instance()->set_fog_color(fog_color);
	}

	void cEnvironmentPrivate::update_sky()
	{
		sRenderer::instance()->set_sky_maps(sky_map ? sky_map->get_view({ 0, 1, 0, 6 }) : nullptr,
			sky_irr_map ? sky_irr_map->get_view({ 0, 1, 0, 6 }) : nullptr,
			sky_rad_map ? sky_rad_map->get_view({ 0, sky_rad_map->n_levels, 0, 6 }) : nullptr);
	}

	void cEnvironmentPrivate::on_active()
	{
		if (environments.empty())
			environments.push_back(this);
		else
		{
			for (auto it = environments.begin(); it != environments.end(); it++)
			{
				if (entity->compare_depth((*it)->entity))
				{
					environments.insert(it, this);
					return;
				}
			}
		}

		if (!environments.empty() && environments.front() == this)
		{
			update_sky();
			sRenderer::instance()->set_sky_intensity(sky_intensity);
			sRenderer::instance()->set_fog_color(fog_color);
		}
	}

	void cEnvironmentPrivate::on_inactive()
	{
		if (!environments.empty() && environments.front() == this)
			sRenderer::instance()->set_sky_maps(nullptr, nullptr, nullptr);
		std::erase_if(environments, [&](auto c) {
			return c == this;
		});
	}

	struct cEnvironmentCreate : cEnvironment::Create
	{
		cEnvironmentPtr operator()(EntityPtr e) override
		{
			return new cEnvironmentPrivate();
		}
	}cEnvironment_create;
	cEnvironment::Create& cEnvironment::create = cEnvironment_create;
}
