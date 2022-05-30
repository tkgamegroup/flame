#include "../../graphics/image.h"
#include "node_private.h"
#include "environment_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cEnvironmentPrivate::~cEnvironmentPrivate()
	{
		node->drawers.remove("environment"_h);
	}

	void cEnvironmentPrivate::on_init()
	{
		node->drawers.add([this](sRendererPtr renderer) {
			draw(renderer);
		}, "environment"_h);
	}

	void cEnvironmentPrivate::set_sky_map_name(const std::filesystem::path& name)
	{
		if (sky_map_name == name)
			return;
		if (!sky_map_name.empty())
		{
			AssetManagemant::release_asset(Path::get(sky_map_name));
			AssetManagemant::release_asset(Path::get(replace_fn(sky_map_name, L"{}_irr")));
			AssetManagemant::release_asset(Path::get(replace_fn(sky_map_name, L"{}_rad")));
		}
		sky_map_name = name;
		if (!sky_map_name.empty())
		{
			AssetManagemant::get_asset(Path::get(sky_map_name));
			AssetManagemant::get_asset(Path::get(replace_fn(sky_map_name, L"{}_irr")));
			AssetManagemant::get_asset(Path::get(replace_fn(sky_map_name, L"{}_rad")));
		}

		auto _sky_map = !sky_map_name.empty() ? graphics::Image::get(sky_map_name) : nullptr;
		if (sky_map != _sky_map)
		{
			if (sky_map)
				graphics::Image::release(sky_map);
			if (sky_map_res_id != -1)
				sRenderer::instance()->release_texture_res(sky_map_res_id);
			sky_map_res_id = sky_map ? sRenderer::instance()->get_texture_res(sky_map->get_view({ 0, 1, 0, 6 }), nullptr, -1) : -1;
			sky_map = _sky_map;
		}
		else if (_sky_map)
			graphics::Image::release(_sky_map);

		node->mark_drawing_dirty();
		data_changed("sky_map_name"_h);
	}

	void cEnvironmentPrivate::draw(sRendererPtr renderer)
	{
		if (sky_map_res_id != -1)
			sRenderer::instance()->set_sky(sky_map_res_id);
	}

	struct cEnvironmentCreate : cEnvironment::Create
	{
		cEnvironmentPtr operator()() override
		{
			return new cEnvironmentPrivate();
		}
	}cEnvironment_create;
	cEnvironment::Create& cEnvironment::create = cEnvironment_create;
}
