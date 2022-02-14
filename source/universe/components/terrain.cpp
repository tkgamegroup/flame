#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../systems/node_renderer_private.h"

namespace flame
{
	cTerrainPrivate::~cTerrainPrivate()
	{
		node->drawers.remove("terrain"_h);
		node->measurers.remove("terrain"_h);

		if (textures)
			graphics::Queue::get(nullptr)->wait_idle();
	}

	void cTerrainPrivate::on_init()
	{
		node->drawers.add([this](sNodeRendererPtr renderer, bool shadow_pass) {
			draw(renderer, shadow_pass);
		}, "mesh"_h);

		node->measurers.add([this](AABB* ret) {
			if (!textures)
				return false;
			*ret = AABB(AABB(node->g_pos, node->g_pos + extent * node->g_scl).get_points(node->transform));
			return true;
		}, "mesh"_h);

		node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_extent(const vec3& _extent)
	{
		if (extent == _extent)
			return;
		extent = _extent;
		apply_src();
		node->mark_drawing_dirty();
		data_changed("extent"_h);
	}

	void cTerrainPrivate::set_blocks(const uvec2& _blocks)
	{
		if (blocks == _blocks)
			return;
		blocks = _blocks;
		node->mark_drawing_dirty();
		data_changed("blocks"_h);
	}

	void cTerrainPrivate::set_tess_level(uint _tess_level)
	{
		if (tess_level == _tess_level)
			return;
		tess_level = _tess_level;
		node->mark_drawing_dirty();
		data_changed("tess_level"_h);
	}

	void cTerrainPrivate::set_texture_name(const std::filesystem::path& _texture_name)
	{
		if (texture_name == _texture_name)
			return;
		texture_name = _texture_name;
		apply_src();
		node->mark_drawing_dirty();
		data_changed("texture_name"_h);
	}

	void cTerrainPrivate::apply_src()
	{
		if (textures)
		{
			graphics::Queue::get(nullptr)->wait_idle();
			textures.reset();
		}

		if (texture_name.empty())
			return;

		auto height_texture = std::unique_ptr<graphics::Image>(graphics::Image::create(nullptr, texture_name));
		if (!height_texture)
			return;

		auto sz0 = (ivec2)height_texture->size;
		textures.reset(graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, sz0, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled, 1, 3));

		auto sz1 = sz0 + 1;

		std::vector<float> heights;
		heights.resize(sz1.x * sz1.y);
		for (auto y = 0; y < sz1.y; y++)
		{
			for (auto x = 0; x < sz1.x; x++)
				heights[y * sz1.x + x] = height_texture->linear_sample(vec2((float)x / sz0.x, (float)y / sz0.y)).x;
		}

		{
			graphics::StagingBuffer nor_stag(nullptr, sizeof(vec4) * sz0.x * sz0.y);
			graphics::StagingBuffer tan_stag(nullptr, sizeof(vec4) * sz0.x * sz0.y);

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
			cb->image_barrier(height_texture.get(), {}, graphics::ImageLayoutTransferSrc);
			cb->image_barrier(textures.get(), {}, graphics::ImageLayoutTransferDst);
			{
				graphics::ImageCopy cpy;
				cpy.size = sz0;
				cb->copy_image(height_texture.get(), textures.get(), { &cpy, 1 });
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

		/*

		if (!material_name.empty())
		{
			auto fn = std::filesystem::path(material_name);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			material = graphics::Material::get(fn.c_str());
			material_id = s_renderer->find_material_res(material);
			if (material_id == -1)
				material_id = s_renderer->set_material_res(-1, material);
		}
		*/
	}

	void cTerrainPrivate::draw(sNodeRendererPtr renderer, bool shadow_pass)
	{
		if (instance_id == -1 || !textures)
			return;

		if (frame < (int)frames)
		{
			renderer->set_terrain_instance(instance_id, node->transform, extent, blocks, tess_level, textures->get_view({ 0, 1, 0, 3 }));
			frame = frames;
		}
		if (!shadow_pass)
			renderer->draw_terrain(instance_id, blocks.x * blocks.y, 0);
	}

	void cTerrainPrivate::on_active()
	{
		instance_id = sNodeRenderer::instance()->register_terrain_instance(-1);

		node->mark_transform_dirty();
	}

	void cTerrainPrivate::on_inactive()
	{
		sNodeRenderer::instance()->register_terrain_instance(instance_id);
		instance_id = -1;
	}

	struct cTerrainCreatePrivate : cTerrain::Create
	{
		cTerrainPtr operator()(EntityPtr e) override
		{
			return new cTerrainPrivate();
		}
	}cTerrain_create_private;
	cTerrain::Create& cTerrain::create = cTerrain_create_private;
}
