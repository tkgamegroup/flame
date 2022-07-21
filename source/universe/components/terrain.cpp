#include "../../graphics/material.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../draw_data.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cTerrainPrivate::~cTerrainPrivate()
	{
		node->drawers.remove("terrain"_h);
		node->measurers.remove("terrain"_h);

		graphics::Queue::get()->wait_idle();
		if (height_map)
			graphics::Image::release(height_map);
		if (normal_map)
			delete normal_map;
		if (tangent_map)
			delete tangent_map;
	}

	void cTerrainPrivate::on_init()
	{
		node->drawers.add([this](DrawData& draw_data) {
			if (instance_id == -1 || !height_map)
				return;

			switch (draw_data.pass)
			{
			case "instance"_h:
				sRenderer::instance()->set_terrain_instance(instance_id, node->transform, extent, blocks, tess_level,
					height_map->get_view(), normal_map->get_view(), tangent_map->get_view(), splash_map->get_view());
				if (grass_field_id != -1)
					sRenderer::instance()->set_grass_field_instance(grass_field_id, grass_field_tess_level);
				break;
			case "draw"_h:
				switch (draw_data.category)
				{
				case "terrain"_h:
					draw_data.terrains.emplace_back(instance_id, product(blocks), material_res_id);
					break;
				case "grass_field"_h:
					if (grass_field_id != -1)
						draw_data.terrains.emplace_back(instance_id, product(blocks), material_res_id);
					break;
				}
				break;
			}
		}, "terrain"_h);
		node->measurers.add([this](AABB* ret) {
			if (!height_map)
				return false;
			*ret = AABB(node->g_pos, node->g_pos + extent * node->g_scl);
			return true;
		}, "terrain"_h);

		node->mark_transform_dirty();
	}


	void cTerrainPrivate::set_extent(const vec3& _extent)
	{
		if (extent == _extent)
			return;
		extent = _extent;

		update_normal_map();

		node->mark_transform_dirty();
		data_changed("extent"_h);
	}

	void cTerrainPrivate::set_blocks(const uvec2& _blocks)
	{
		if (blocks == _blocks)
			return;
		blocks = _blocks;

		update_normal_map();

		node->mark_transform_dirty();
		data_changed("blocks"_h);
	}

	void cTerrainPrivate::set_tess_level(uint _tess_level)
	{
		if (tess_level == _tess_level)
			return;
		tess_level = _tess_level;

		update_normal_map();

		node->mark_transform_dirty();
		data_changed("tess_level"_h);
	}

	void cTerrainPrivate::set_height_map_name(const std::filesystem::path& name)
	{
		if (height_map_name == name)
			return;
		if (!height_map_name.empty())
			AssetManagemant::release_asset(Path::get(height_map_name));
		height_map_name = name;
		if (!height_map_name.empty())
			AssetManagemant::get_asset(Path::get(height_map_name));

		auto _height_map = !height_map_name.empty() ? graphics::Image::get(height_map_name, false, false, 0.f, graphics::ImageUsageAttachment) : nullptr;
		if (height_map != _height_map)
		{
			if (height_map)
				graphics::Image::release(height_map);
			height_map = _height_map;
			update_normal_map();
		}
		else if (_height_map)
			graphics::Image::release(_height_map);

		node->mark_transform_dirty();
		data_changed("height_map_name"_h);
	}

	void cTerrainPrivate::set_splash_map_name(const std::filesystem::path& name)
	{
		if (splash_map_name == name)
			return;
		if (!splash_map_name.empty())
			AssetManagemant::release_asset(Path::get(splash_map_name));
		splash_map_name = name;
		if (!splash_map_name.empty())
			AssetManagemant::get_asset(Path::get(splash_map_name));

		auto _splash_map = !splash_map_name.empty() ? graphics::Image::get(splash_map_name, false, false, 0.f, graphics::ImageUsageAttachment) : nullptr;
		if (splash_map != _splash_map)
		{
			if (splash_map)
				graphics::Image::release(splash_map);
			splash_map = _splash_map;
			update_normal_map();
		}
		else if (_splash_map)
			graphics::Image::release(_splash_map);

		node->mark_transform_dirty();
		data_changed("splash_map_name"_h);
	}

	void cTerrainPrivate::set_material_name(const std::filesystem::path& name)
	{
		if (material_name == name)
			return;
		material_name = name;

		auto _material = !material_name.empty() ? graphics::Material::get(material_name) : nullptr;
		if (material != _material)
		{
			if (material_res_id != -1)
				sRenderer::instance()->release_material_res(material_res_id);
			if (material)
				graphics::Material::release(material);
			material = _material;
			material_res_id = material ? sRenderer::instance()->get_material_res(material, -1) : -1;
		}
		else if (_material)
			graphics::Material::release(_material);

		node->mark_drawing_dirty();
		data_changed("material_name"_h);
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

		normal_map = graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, sz0, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled);
		tangent_map = graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, sz0, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled);

		std::vector<float> heights;
		heights.resize(sz1.x * sz1.y);
		for (auto y = 0; y < sz1.y; y++)
		{
			for (auto x = 0; x < sz1.x; x++)
				heights[y * sz1.x + x] = height_map->linear_sample(vec2((float)x / sz0.x, (float)y / sz0.y)).x;
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
		cpy.img_ext = sz0;
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
		if (use_grass_field)
			grass_field_id = sRenderer::instance()->register_grass_field_instance(-1);

		node->mark_transform_dirty();
	}

	void cTerrainPrivate::on_inactive()
	{
		sRenderer::instance()->register_terrain_instance(instance_id);
		if (grass_field_id != -1)
			sRenderer::instance()->register_grass_field_instance(grass_field_id);
		instance_id = -1;
		grass_field_id = -1;

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
