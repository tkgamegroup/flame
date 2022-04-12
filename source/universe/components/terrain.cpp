#include "../../graphics/material.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cTerrainPrivate::~cTerrainPrivate()
	{
		node->drawers.remove("terrain"_h);
		node->measurers.remove("terrain"_h);

		if (textures || height_map)
		{
			graphics::Queue::get()->wait_idle();
			if (height_map)
				graphics::Image::release(height_map);
		}
	}

	void cTerrainPrivate::on_init()
	{
		node->drawers.add([this](sRendererPtr renderer) {
			draw(renderer);
		}, "mesh"_h);

		node->measurers.add([this](AABB* ret) {
			if (!textures)
				return false;
			*ret = AABB(node->g_pos, node->g_pos + extent * node->g_scl);
			return true;
		}, "mesh"_h);

		node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_extent(const vec3& _extent)
	{
		if (extent == _extent)
			return;
		extent = _extent;

		build_textures();

		node->mark_transform_dirty();
		data_changed("extent"_h);
	}

	void cTerrainPrivate::set_blocks(const uvec2& _blocks)
	{
		if (blocks == _blocks)
			return;
		blocks = _blocks;

		build_textures();

		node->mark_transform_dirty();
		data_changed("blocks"_h);
	}

	void cTerrainPrivate::set_tess_level(uint _tess_level)
	{
		if (tess_level == _tess_level)
			return;
		tess_level = _tess_level;

		build_textures();

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

		auto _height_map = !height_map_name.empty() ? graphics::Image::get(height_map_name) : nullptr;
		if (height_map != _height_map)
		{
			if (height_map)
				graphics::Image::release(height_map);
			height_map = _height_map;
			build_textures();
		}
		else if (_height_map)
			graphics::Image::release(_height_map);

		node->mark_transform_dirty();
		data_changed("height_map_name"_h);
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
			material_res_id = material ? sRenderer::instance()->get_material_res(material) : -1;
		}
		else if (_material)
			graphics::Material::release(_material);

		node->mark_drawing_dirty();
		data_changed("material_name"_h);
	}

	void cTerrainPrivate::build_textures()
	{
		if (textures)
		{
			graphics::Queue::get()->wait_idle();
			textures.reset();
		}

		if (!height_map)
			return;

		auto sz0 = (ivec2)height_map->size;
		auto sz1 = sz0 + 1;

		textures.reset(graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, sz0, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 3));

		std::vector<float> heights;
		heights.resize(sz1.x * sz1.y);
		for (auto y = 0; y < sz1.y; y++)
		{
			for (auto x = 0; x < sz1.x; x++)
				heights[y * sz1.x + x] = height_map->linear_sample(vec2((float)x / sz0.x, (float)y / sz0.y)).x;
		}

		{
			graphics::StagingBuffer nor_stag(sizeof(vec4) * sz0.x * sz0.y);
			graphics::StagingBuffer tan_stag(sizeof(vec4) * sz0.x * sz0.y);

			auto nor_dat = (cvec4*)nor_stag->mapped;
			auto tan_dat = (cvec4*)tan_stag->mapped;

			auto h = extent.y * (extent.x / sz0.x);
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
					n += 1.f;
					n *= 0.5f;
					*nor_dat++ = cvec4(n * 255.f, 255);

					auto t = normalize(vec3(+1.f, hR, 0.f) - vec3(-1.f, hL, 0.f));
					t += 1.f;
					t *= 0.5f;
					*tan_dat++ = cvec4(t * 255.f, 255);
				}
			}

			graphics::InstanceCB cb(nullptr);
			graphics::BufferImageCopy cpy;
			cpy.img_ext = sz0;
			cb->image_barrier(height_map, {}, graphics::ImageLayoutTransferSrc);
			cb->image_barrier(textures.get(), {}, graphics::ImageLayoutTransferDst);
			{
				graphics::ImageCopy cpy;
				cpy.size = sz0;
				cb->copy_image(height_map, textures.get(), { &cpy, 1 });
			}
			cb->image_barrier(textures.get(), {}, graphics::ImageLayoutShaderReadOnly);

			cpy.img_sub = { 0, 1, 1, 1 };
			cb->image_barrier(textures.get(), cpy.img_sub, graphics::ImageLayoutTransferDst);
			cb->copy_buffer_to_image(nor_stag.get(), textures.get(), { &cpy, 1 } );
			cb->image_barrier(textures.get(), cpy.img_sub, graphics::ImageLayoutShaderReadOnly);

			cpy.img_sub = { 0, 1, 2, 1 };
			cb->image_barrier(textures.get(), cpy.img_sub, graphics::ImageLayoutTransferDst);
			cb->copy_buffer_to_image(tan_stag.get(), textures.get(), { &cpy, 1 });
			cb->image_barrier(textures.get(), cpy.img_sub, graphics::ImageLayoutShaderReadOnly);
		}
	}

	/*
	if (!material_name.empty())
	{
		auto fn = std::filesystem::path(material_name);
		if (!fn.extension().empty() && !fn.is_absolute())
			fn = parent_path / fn;
		material = graphics::Material::get(fn.c_str());
		material_id = s_renderer->find_material_res(material);
		if (material_id == -1)
			material_id = s_renderer->set_material_res(-1, material);
	}
	*/

	void cTerrainPrivate::draw(sRendererPtr renderer)
	{
		if (instance_id == -1 || !textures || material_res_id == -1)
			return;

		if (frame < (int)frames)
		{
			renderer->set_terrain_instance(instance_id, node->transform, extent, blocks, tess_level, textures->get_view({ 0, 1, 0, 3 }));
			frame = frames;
		}

		renderer->draw_terrain(instance_id, blocks.x * blocks.y, material_res_id);
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
