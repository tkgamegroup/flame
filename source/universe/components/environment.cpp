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

		apply();
		data_changed("sky_map_name"_h);
	}

	void cEnvironmentPrivate::apply()
	{
		if (!environments.empty() && environments.front() == this)
		{
			sRenderer::instance()->dirty = true;
			sRenderer::instance()->set_sky(sky_map ? sky_map->get_view({ 0, 1, 0, 6 }) : nullptr, nullptr, nullptr);
		}
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
		apply();
	}

	void cEnvironmentPrivate::on_inactive()
	{
		if (!environments.empty() && environments.front() == this)
			sRenderer::instance()->set_sky(nullptr, nullptr, nullptr);
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
