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

		auto old_one = data_map;
		if (!data_map_name.empty())
		{
			if (!data_map_name.native().starts_with(L"0x"))
				AssetManagemant::release_asset(Path::get(data_map_name));
			else
				old_one = nullptr;
		}
		data_map_name = name;
		if (!data_map_name.empty())
		{
			if (!data_map_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get_asset(Path::get(data_map_name));
				data_map = !data_map_name.empty() ? graphics::Image::get(data_map_name, false, false, 0.f, graphics::ImageUsageAttachment | graphics::ImageUsageStorage) : nullptr;
			}
			else
				data_map = (graphics::ImagePtr)s2u_hex<uint64>(data_map_name.string());
		}

		if (data_map != old_one)
		{
			dirty = true;
			node->mark_transform_dirty();
		}

		if (old_one)
			graphics::Image::release(old_one);
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
					sRenderer::instance()->set_volume_instance(instance_id, node->transform, extent, blocks, data_map->get_view());
					dirty = false;
				}
				break;
			case PassGBuffer:
				if (marching_cubes)
				{
					if (draw_data.categories & CateMarchingCubes)
						draw_data.volumes.emplace_back(instance_id, blocks, 0);
				}
				else
				{
					if (draw_data.categories & CateVolume)
						draw_data.volumes.emplace_back(instance_id, blocks, 0);
				}
				break;
			}
		}, "volume"_h);
		node->measurers.add([this](AABB* ret) {
			if (!data_map)
				return false;
			*ret = AABB(node->g_pos, node->g_pos + extent * node->g_scl);
			return true;
		}, "volume"_h);
		node->data_listeners.add([this](uint hash) {
			if (hash == "transform"_h)
			dirty = true;
		}, "volume"_h);
		data_listeners.add([this](uint hash) {
			if (hash == "enable"_h)
			dirty = true;
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
