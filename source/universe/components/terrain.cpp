#include "../../graphics/material.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cTerrainPrivate::set_extent(const vec3& _extent)
	{
		if (extent == _extent)
			return;
		extent = _extent;

		update_normal_map();

		dirty = true;
		node->mark_transform_dirty();
		data_changed("extent"_h);
	}

	void cTerrainPrivate::set_blocks(const uvec2& _blocks)
	{
		if (blocks == _blocks)
			return;
		blocks = _blocks;

		update_normal_map();

		dirty = true;
		node->mark_transform_dirty();
		data_changed("blocks"_h);
	}

	void cTerrainPrivate::set_tess_level(uint _tess_level)
	{
		if (tess_level == _tess_level)
			return;
		tess_level = _tess_level;

		update_normal_map();

		dirty = true;
		node->mark_transform_dirty();
		data_changed("tess_level"_h);
	}

	void cTerrainPrivate::set_height_map_name(const std::filesystem::path& name)
	{
		if (height_map_name == name)
			return;

		auto old_one = height_map;
		auto old_raw = !height_map_name.empty() && height_map_name.native().starts_with(L"0x");
		if (!height_map_name.empty())
		{
			if (!old_raw)
				AssetManagemant::release(Path::get(height_map_name));
		}
		height_map_name = name;
		height_map = nullptr;
		if (!height_map_name.empty())
		{
			if (!height_map_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get(Path::get(height_map_name));
				height_map = !height_map_name.empty() ? graphics::Image::get(height_map_name) : nullptr;
				if (height_map && (height_map->usage & graphics::ImageUsageAttachment) == 0)
				{
					printf("The height map used by terrain must have attachement usage\n");
					height_map = nullptr;
				}
			}
			else
				height_map = (graphics::ImagePtr)s2u_hex<uint64>(height_map_name.string());
		}

		if (height_map != old_one)
		{
			dirty = true;
			update_normal_map();
			node->mark_transform_dirty();
		}

		if (!old_raw && old_one)
			graphics::Image::release(old_one);
		data_changed("height_map_name"_h);
	}

	void cTerrainPrivate::set_material_name(const std::filesystem::path& name)
	{
		if (material_name == name)
			return;

		auto old_one = material;
		auto old_raw = !material_name.empty() && material_name.native().starts_with(L"0x");
		if (!material_name.empty())
		{
			if (!old_raw)
				AssetManagemant::release(Path::get(material_name));
		}
		material_name = name;
		material = nullptr;
		if (!material_name.empty())
		{
			if (!material_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get(Path::get(material_name));
				material = !material_name.empty() ? graphics::Material::get(material_name) : nullptr;
			}
			else
				material = (graphics::MaterialPtr)s2u_hex<uint64>(material_name.string());
		}

		if (material != old_one)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			material_res_id = material ? sRenderer::instance()->get_material_res(material, -1) : -1;
		}
		if (!old_raw && old_one)
			graphics::Material::release(old_one);

		node->mark_drawing_dirty();
		data_changed("material_name"_h);
	}

	void cTerrainPrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow == v)
			return;
		cast_shadow = v;

		dirty = true;
		data_changed("cast_shadow"_h);
	}

	void cTerrainPrivate::set_use_grass_field(bool v)
	{
		if (use_grass_field == v)
			return;
		use_grass_field = v;

		dirty = true;
		data_changed("use_grass_field"_h);
	}

	void cTerrainPrivate::set_grass_field_tess_level(uint tess_level)
	{
		if (grass_field_tess_level == tess_level)
			return;
		grass_field_tess_level = tess_level;

		dirty = true;
		data_changed("grass_field_tess_level"_h);
	}

	void cTerrainPrivate::set_grass_channel(uint channel)
	{
		if (grass_channel == channel)
			return;
		grass_channel = channel;

		dirty = true;
		data_changed("grass_channel"_h);
	}

	void cTerrainPrivate::set_grass_texture_name(const std::filesystem::path& name)
	{
		if (grass_texture_name == name)
			return;
		if (!grass_texture_name.empty())
			AssetManagemant::release(Path::get(grass_texture_name));
		grass_texture_name = name;
		if (!grass_texture_name.empty())
			AssetManagemant::get(Path::get(grass_texture_name));

		auto _texture = !grass_texture_name.empty() ? graphics::Image::get(grass_texture_name) : nullptr;
		if (grass_texture != _texture)
		{
			if (grass_texture_id != -1)
				sRenderer::instance()->release_texture_res(grass_texture_id);
			if (grass_texture)
				graphics::Image::release(grass_texture);
			grass_texture = _texture;
			grass_texture_id = grass_texture ? sRenderer::instance()->get_texture_res(grass_texture->get_view({ 0, grass_texture->n_levels, 0, 1 }), nullptr, -1) : -1;
		}
		else if (_texture)
			graphics::Image::release(_texture);

		dirty = true;
		node->mark_drawing_dirty();
		data_changed("grass_texture_name"_h);
	}

	cTerrainPrivate::~cTerrainPrivate()
	{
		node->drawers.remove("terrain"_h);
		node->measurers.remove("terrain"_h);
		node->data_listeners.remove("terrain"_h);

		graphics::Queue::get()->wait_idle();
		if (material_res_id != -1)
			sRenderer::instance()->release_material_res(material_res_id);
		if (grass_texture_id != -1)
			sRenderer::instance()->release_texture_res(grass_texture_id);
		if (!height_map_name.empty() && !height_map_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(height_map_name));
			if (height_map)
				graphics::Image::release(height_map);
		}
		if (!material_name.empty() && !material_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(material_name));
			if (material)
				graphics::Material::release(material);
		}
		if (!grass_texture_name.empty())
			AssetManagemant::release(Path::get(grass_texture_name));
		if (grass_texture)
			graphics::Image::release(grass_texture);
		if (normal_map)
			delete normal_map;
		if (tangent_map)
			delete tangent_map;
	}

	void cTerrainPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data, cCameraPtr camera) {
			if (instance_id == -1 || material_res_id == -1 ||  !height_map)
				return;

			switch (draw_data.pass)
			{
			case PassInstance:
				if (dirty)
				{
					sRenderer::instance()->set_terrain_instance(instance_id, node->transform, extent, blocks, tess_level, grass_field_tess_level, grass_channel, grass_texture_id,
						height_map->get_view(), normal_map->get_view(), tangent_map->get_view());
					dirty = false;
				}
				break;
			case PassGBuffer:
				if ((draw_data.categories & CateTerrain))
					draw_data.terrains.emplace_back(instance_id, blocks, material_res_id);
				break;
			case PassForward:
				if (use_grass_field)
				{
					if ((draw_data.categories & CateGrassField))
						draw_data.terrains.emplace_back(instance_id, blocks, material_res_id);
				}
				break;
			case PassOcculder:
				if ((draw_data.categories & CateTerrain) && cast_shadow)
					draw_data.terrains.emplace_back(instance_id, blocks, material_res_id);
				break;
			case PassPickUp:
				if ((draw_data.categories & CateTerrain))
					draw_data.terrains.emplace_back(instance_id, blocks, material_res_id);
				break;
			}
		}, "terrain"_h);
		node->measurers.add([this](AABB& b) {
			if (height_map)
			{
				auto pos = node->global_pos();
				b.expand(AABB(pos, pos + extent * node->global_scl()));
			}
		}, "terrain"_h);
		node->data_listeners.add([this](uint hash) {
			if (hash == "transform"_h)
				dirty = true;
		}, "terrain"_h);
		data_listeners.add([this](uint hash) {
			if (hash == "enable"_h)
				dirty = true;
		}, "terrain"_h);

		node->mark_transform_dirty();
	}

	void cTerrainPrivate::update_normal_map()
	{
		if (!height_map)
			return;

		graphics::Queue::get()->wait_idle();

		if (normal_map)
			delete normal_map;
		if (tangent_map)
			delete tangent_map;

		auto sz0 = (ivec2)(blocks * tess_level);
		auto sz1 = sz0 + 1;

		normal_map = graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(sz0, 1), graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled);
		tangent_map = graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, uvec3(sz0, 1), graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled);
		dirty = true;

		std::vector<float> heights;
		heights.resize(sz1.x * sz1.y);
		for (auto y = 0; y < sz1.y; y++)
		{
			for (auto x = 0; x < sz1.x; x++)
				heights[y * sz1.x + x] = height_map->linear_sample_staging_pixels(vec2((float)x / sz0.x, (float)y / sz0.y)).x;
		}

		graphics::StagingBuffer nor_stag(sizeof(cvec4) * sz0.x * sz0.y);
		graphics::StagingBuffer tan_stag(sizeof(cvec4) * sz0.x * sz0.y);

		auto nor_dat = (cvec4*)nor_stag->mapped;
		auto tan_dat = (cvec4*)tan_stag->mapped;

		auto h = extent.y / (extent.x / sz0.x);
		for (auto y = 0; y < sz0.y; y++)
		{
			for (auto x = 0; x < sz0.x; x++)
			{
				auto LT = heights[y * sz1.x + x];
				auto RT = heights[y * sz1.x + x + 1];
				auto LB = heights[(y + 1) * sz1.x + x];
				auto RB = heights[(y + 1) * sz1.x + x + 1];

				float hL = (LT + LB) * 0.5f * h;
				float hR = (RT + RB) * 0.5f * h;
				float hU = (LT + RT) * 0.5f * h;
				float hD = (LB + RB) * 0.5f * h;

				auto n = vec3(hL - hR, 2.f, hU - hD);
				n = normalize(n);
				n = (n + 1.f) * 0.5f;
				*nor_dat++ = cvec4(n * 255.f, 255);

				auto t = normalize(vec3(+1.f, hR, 0.f) - vec3(-1.f, hL, 0.f));
				t = normalize(t);
				t = (t + 1.f) * 0.5f;
				*tan_dat++ = cvec4(t * 255.f, 255);
			}
		}

		graphics::InstanceCommandBuffer cb(nullptr);
		graphics::BufferImageCopy cpy;
		cpy.img_ext = uvec3(sz0, 1);
		cb->image_barrier(normal_map, cpy.img_sub, graphics::ImageLayoutTransferDst);
		cb->copy_buffer_to_image(nor_stag.get(), normal_map, { &cpy, 1 });
		cb->image_barrier(normal_map, cpy.img_sub, graphics::ImageLayoutShaderReadOnly);
		cb->image_barrier(tangent_map, cpy.img_sub, graphics::ImageLayoutTransferDst);
		cb->copy_buffer_to_image(tan_stag.get(), tangent_map, { &cpy, 1 });
		cb->image_barrier(tangent_map, cpy.img_sub, graphics::ImageLayoutShaderReadOnly);
		cb.excute();
	}

	void cTerrainPrivate::on_active()
	{
		instance_id = sRenderer::instance()->register_terrain_instance(-1);

		node->mark_transform_dirty();
	}

	void cTerrainPrivate::on_inactive()
	{
		sRenderer::instance()->register_terrain_instance(instance_id);
		instance_id = -1;

		node->mark_transform_dirty();
	}

	struct cTerrainCreate : cTerrain::Create
	{
		cTerrainPtr operator()(EntityPtr e) override
		{
			return new cTerrainPrivate();
		}
	}cTerrain_create;
	cTerrain::Create& cTerrain::create = cTerrain_create;
}
