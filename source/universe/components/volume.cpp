#include "node_private.h"
#include "volume_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cVolumePrivate::set_extent(const vec3& _extent)
	{
		if (extent == _extent)
			return;
		extent = _extent;

		dirty = true;
		node->mark_transform_dirty();
		data_changed("extent"_h);
	}

	void cVolumePrivate::set_blocks(const uvec3& _blocks)
	{
		if (blocks == _blocks)
			return;
		blocks = _blocks;

		dirty = true;
		node->mark_transform_dirty();
		data_changed("blocks"_h);
	}

	void cVolumePrivate::set_data_map_name(const std::filesystem::path& name)
	{
		if (data_map_name == name)
			return;
		if (!data_map_name.empty())
			AssetManagemant::release_asset(Path::get(data_map_name));
		data_map_name = name;
		if (!data_map_name.empty())
			AssetManagemant::get_asset(Path::get(data_map_name));

		auto _data_map = !data_map_name.empty() ? graphics::Image::get(data_map_name, false, false, 0.f, graphics::ImageUsageAttachment | graphics::ImageUsageStorage) : nullptr;
		if (data_map != _data_map)
		{
			if (data_map)
				graphics::Image::release(data_map);
			data_map = _data_map;
		}
		else if (_data_map)
			graphics::Image::release(_data_map);

		dirty = true;
		node->mark_transform_dirty();
		data_changed("data_map_name"_h);
	}

	cVolumePrivate::~cVolumePrivate()
	{
		node->drawers.remove("volume"_h);
		node->measurers.remove("volume"_h);
	}

	void cVolumePrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (instance_id == -1 || !data_map)
				return;

			switch (draw_data.pass)
			{
			case PassInstance:
				if (dirty)
				{
					sRenderer::instance()->set_volume_instance(instance_id, node->transform, extent, blocks, data_map->get_view({}, { graphics::SwizzleR, graphics::SwizzleR, graphics::SwizzleR, graphics::SwizzleR }));
					dirty = false;
				}
				break;
			case PassGBuffer:
				if (marching_cubes)
				{
					if (draw_data.categories & CateMarchingCubes)
						draw_data.volumes.emplace_back(instance_id, blocks.x, 0);
				}
				else
				{
					if (draw_data.categories & CateVolume)
						draw_data.volumes.emplace_back(instance_id, blocks.x, 0);
				}
				break;
			}
		}, "volume"_h);
		node->measurers.add([this](AABB* ret) {
			*ret = AABB(AABB(vec3(0.f), 10.f).get_points(node->transform));
			return true;
		}, "volume"_h);

		node->mark_transform_dirty();
	}

	void cVolumePrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_volume_instance(-1);

		node->mark_transform_dirty();
	}

	void cVolumePrivate::on_inactive()
	{
		sRenderer::instance()->register_volume_instance(instance_id);
		instance_id = -1;
	}

	struct cVolumeCreate : cVolume::Create
	{
		cVolumePtr operator()(EntityPtr e) override
		{
			return new cVolumePrivate();
		}
	}cVolume_create;
	cVolume::Create& cVolume::create = cVolume_create;
}
