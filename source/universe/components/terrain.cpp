#include "../../graphics/device.h"
#include "../../graphics/buffer.h"
#include "../../graphics/image.h"
#include "../../graphics/material.h"
#include "../entity_private.h"
#include "../world_private.h"
#include "node_private.h"
#include "terrain_private.h"
#include "../systems/renderer_private.h"

namespace flame
{
	void cTerrainPrivate::set_extent(const vec2& ext)
	{
		if (extent == ext)
			return;
		extent = ext;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_blocks(uint b)
	{
		if (blocks == b)
			return;
		blocks = b;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_tess_levels(uint l)
	{
		if (tess_levels == l)
			return;
		tess_levels = l;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::set_height_map(std::string_view name)
	{
		if (height_map_name == name)
			return;
		height_map_name = name;
	}

	void cTerrainPrivate::set_material_name(std::string_view name)
	{
		if (material_name == name)
			return;
		material_name = name;
	}

	void cTerrainPrivate::set_shading_flags(ShadingFlags flags)
	{
		if (shading_flags == flags)
			return;
		shading_flags = flags;
		if (node)
			node->mark_transform_dirty();
	}

	void cTerrainPrivate::draw(sRendererPtr s_renderer, bool first, bool)
	{
		if (!first || height_map_id == -1 || material_id == -1)
			return;

		s_renderer->draw_terrain(node->g_pos, vec3(extent.x, extent.y, extent.x), uvec2(blocks), tess_levels, height_map_id, 
			normal_map_id, tangent_map_id, material_id, shading_flags);
	}

	void cTerrainPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		assert(node);

		node->mark_drawing_dirty();
	}

	void cTerrainPrivate::on_removed()
	{
		node = nullptr;
	}

	void cTerrainPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		assert(s_renderer);

		auto ppath = entity->get_src(src_id).parent_path();

		{
			auto fn = std::filesystem::path(height_map_name);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			height_texture = graphics::Image::get(nullptr, fn.c_str(), false);
			auto view = height_texture->get_view();
			height_map_id = s_renderer->find_texture_res(view);
			if (height_map_id == -1)
				height_map_id = s_renderer->set_texture_res(-1, view, nullptr);
		}
		{
			auto tex_size = height_texture->get_size();
			assert(tex_size.x == tex_size.y);
			auto s = tex_size.x;
			auto s1 = s + 1;

			normal_texture.reset(graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, tex_size, 1, 1,
				graphics::SampleCount_1, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
			tangent_texture.reset(graphics::Image::create(nullptr, graphics::Format_R8G8B8A8_UNORM, tex_size, 1, 1,
				graphics::SampleCount_1, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
			normal_map_id = s_renderer->set_texture_res(-1, normal_texture->get_view(), nullptr);
			tangent_map_id = s_renderer->set_texture_res(-1, tangent_texture->get_view(), nullptr);

			std::vector<float> heights;
			heights.resize(s1 * s1);
			auto pres = heights.data();
			for (auto y = 0; y < s1; y++)
			{
				for (auto x = 0; x < s1; x++)
					*pres++ = height_texture->linear_sample(vec2((float)x / s, (float)y / s)).x;
			}

			auto img_sz = sizeof(vec4) * s * s;
			graphics::StagingBuffer nor_stag(nullptr, img_sz, nullptr, graphics::BufferUsageTransferDst);
			graphics::StagingBuffer tan_stag(nullptr, img_sz, nullptr, graphics::BufferUsageTransferDst);
			auto nor_dat = (cvec4*)nor_stag.mapped;
			auto tan_dat = (cvec4*)tan_stag.mapped;
			auto h = extent.y * (extent.x / s);
			for (auto y = 0; y < s; y++)
			{
				for (auto x = 0; x < s; x++)
				{
					auto LT = heights[y * s1 + x];
					auto RT = heights[y * s1 + x + 1];
					auto LB = heights[(y + 1) * s1 + x];
					auto RB = heights[(y + 1) * s1 + x + 1];

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
			cpy.img_ext = tex_size;
			cb->image_barrier(normal_texture.get(), {}, graphics::ImageLayoutUndefined, graphics::ImageLayoutTransferDst);
			cb->copy_buffer_to_image(nor_stag.get(), normal_texture.get(), 1, &cpy);
			cb->image_barrier(normal_texture.get(), {}, graphics::ImageLayoutTransferDst, graphics::ImageLayoutShaderReadOnly);
			cb->image_barrier(tangent_texture.get(), {}, graphics::ImageLayoutUndefined, graphics::ImageLayoutTransferDst);
			cb->copy_buffer_to_image(tan_stag.get(), tangent_texture.get(), 1, &cpy);
			cb->image_barrier(tangent_texture.get(), {}, graphics::ImageLayoutTransferDst, graphics::ImageLayoutShaderReadOnly);
		}

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
	}

	void cTerrainPrivate::on_left_world()
	{
		s_renderer = nullptr;
	}

	cTerrain* cTerrain::create()
	{
		return new cTerrainPrivate();
	}
}
