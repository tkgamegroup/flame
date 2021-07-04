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

	void cTerrainPrivate::set_height_map(const std::string& name)
	{
		if (height_map_name == name)
			return;
		height_map_name = name;
	}

	void cTerrainPrivate::set_material_name(const std::string& name)
	{
		if (material_name == name)
			return;
		material_name = name;
	}

	void cTerrainPrivate::draw(sRendererPtr s_renderer)
	{
		if (height_map_id == -1 || material_id == -1)
			return;

		s_renderer->draw_terrain(node, vec3(extent.x, extent.y, extent.x), uvec2(blocks), tess_levels, height_map_id, normal_map_id,
			material_id, shading_flags);
	}

	void cTerrainPrivate::on_added()
	{
		node = entity->get_component_i<cNodePrivate>(0);
		fassert(node);

		node->mark_drawing_dirty();
	}

	void cTerrainPrivate::on_removed()
	{
		node = nullptr;
	}

	void cTerrainPrivate::on_entered_world()
	{
		s_renderer = entity->world->get_system_t<sRendererPrivate>();
		fassert(s_renderer);

		auto device = graphics::Device::get_default();
		auto ppath = entity->get_src(src_id).parent_path();

		{
			auto fn = std::filesystem::path(height_map_name);
			if (!fn.extension().empty() && !fn.is_absolute())
				fn = ppath / fn;
			height_texture = graphics::Image::get(device, fn.c_str(), false);
			auto view = height_texture->get_view();
			height_map_id = s_renderer->find_texture_res(view);
			if (height_map_id == -1)
				height_map_id = s_renderer->set_texture_res(-1, view, nullptr);
		}
		{
			auto tex_size = height_texture->get_size();
			fassert(tex_size.x == tex_size.y);
			normal_texture.reset(graphics::Image::create(device, graphics::Format_R8G8B8A8_UNORM, tex_size, 1, 1, 
				graphics::SampleCount_1, graphics::ImageUsageTransferSrc | graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
			normal_map_id = s_renderer->set_texture_res(-1, normal_texture->get_view(), nullptr);

			std::vector<vec4> res;
			res.resize((tex_size.x + 1) * (tex_size.y + 1));
			height_texture->grid_sample(vec4(vec2(0.f), vec2(1.f) / vec2(tex_size)), tex_size + 1U, res.data());

			graphics::StagingBuffer stag(device, sizeof(vec4) * tex_size.x * tex_size.y, nullptr, graphics::BufferUsageTransferDst);
			auto nor_dat = (cvec4*)stag.mapped;
			auto h = extent.y * (extent.x / tex_size.x);
			auto ln = tex_size.x + 1;
			for (auto y = 0; y < tex_size.y; y++)
			{
				for (auto x = 0; x < tex_size.x; x++)
				{
					auto LT = res[y * ln + x].x;
					auto RT = res[y * ln + x + 1].x;
					auto LB = res[(y + 1) * ln + x].x;
					auto RB = res[(y + 1) * ln + x + 1].x;

					float hL = (LT + LB) * 0.5f * h;
					float hR = (RT + RB) * 0.5f * h;
					float hU = (LT + RT) * 0.5f * h;
					float hD = (LB + RB) * 0.5f * h;

					auto n = vec3(hL - hR, 2.f, hU - hD);
					n = normalize(n);
					n += 1.f;
					n *= 0.5f;
					*nor_dat++ = cvec4(n * 255.f, 255);
				}
			}

			graphics::InstanceCB cb(device);
			cb->image_barrier(normal_texture.get(), {}, graphics::ImageLayoutUndefined, graphics::ImageLayoutTransferDst);
			graphics::BufferImageCopy cpy;
			cpy.img_ext = tex_size;
			cb->copy_buffer_to_image(stag.get(), normal_texture.get(), 1, &cpy);
			cb->image_barrier(normal_texture.get(), {}, graphics::ImageLayoutTransferDst, graphics::ImageLayoutShaderReadOnly);
		}
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

	cTerrain* cTerrain::create(void* parms)
	{
		return new cTerrainPrivate();
	}
}
