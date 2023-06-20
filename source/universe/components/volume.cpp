#include "../../graphics/material.h"
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
		auto old_raw = !data_map_name.empty() && data_map_name.native().starts_with(L"0x");
		if (!data_map_name.empty())
		{
			if (!old_raw)
				AssetManagemant::release(Path::get(data_map_name));
		}
		data_map_name = name;
		data_map = nullptr;
		if (!data_map_name.empty())
		{
			if (!data_map_name.native().starts_with(L"0x"))
			{
				AssetManagemant::get(Path::get(data_map_name));
				data_map = !data_map_name.empty() ? graphics::Image::get(data_map_name) : nullptr;
				if (data_map && ((data_map->usage & graphics::ImageUsageAttachment) == 0 || (data_map->usage & graphics::ImageUsageAttachment) == 0))
				{
					printf("The data map used by volumn must have attachement and storage usage\n");
					data_map = nullptr;
				}
			}
			else
				data_map = (graphics::ImagePtr)s2u_hex<uint64>(data_map_name.string());
		}

		if (data_map != old_one)
		{
			dirty = true;
			update_height_and_normal_map();
			node->mark_transform_dirty();
		}

		if (!old_raw && old_one)
			graphics::Image::release(old_one);
		data_changed("data_map_name"_h);
	}

	void cVolumePrivate::set_material_name(const std::filesystem::path& name)
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

	void cVolumePrivate::set_cast_shadow(bool v)
	{
		if (cast_shadow == v)
			return;
		cast_shadow = v;

		dirty = true;
		data_changed("cast_shadow"_h);
	}

	cVolumePrivate::~cVolumePrivate()
	{
		node->drawers.remove("volume"_h);
		node->measurers.remove("volume"_h);

		graphics::Queue::get()->wait_idle();
		if (material_res_id != -1)
			sRenderer::instance()->release_material_res(material_res_id);
		if (!data_map_name.empty() && !data_map_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(data_map_name));
			if (data_map)
				graphics::Image::release(data_map);
		}
		if (!material_name.empty() && !material_name.native().starts_with(L"0x"))
		{
			AssetManagemant::release(Path::get(material_name));
			if (material)
				graphics::Material::release(material);
		}
	}

#include "../systems/marching_cubes_lookup.h"

	void cVolumePrivate::update_height_and_normal_map()
	{
		if (!data_map)
			return;

		graphics::Queue::get()->wait_idle();

		if (height_map)
			delete height_map;
		if (normal_map)
			delete normal_map;
		if (tangent_map)
			delete tangent_map;

		height_map = graphics::Image::create(graphics::Format_Depth16, uvec3(data_map->extent.x, data_map->extent.z, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageTransferSrc);
		{
			graphics::InstanceCommandBuffer cb;

			auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\volume\\get_height.pipeline", { "rp:dep_fmt=" + TypeInfo::serialize_t(height_map->format),
				"all_shader:CUSTOM_INPUT",
				"all_shader:_transform=pc.transform",
				"all_shader:_proj_view=pc.proj_view",
				"all_shader:_extent=pc.extent",
				"all_shader:_blocks=pc.blocks",
				"all_shader:DATA_MAP=volume_data",
				});
			auto prm = graphics::PipelineResourceManager(pl->layout, graphics::PipelineGraphics);
			auto dsl = prm.get_dsl(""_h);
			graphics::StorageBuffer buf_marching_cubes_loopup(graphics::BufferUsageStorage, dsl->get_buf_ui("MarchingCubesLookup"_h));
			{
				auto item = buf_marching_cubes_loopup.mark_dirty_c("items"_h);
				auto p = item.data;
				assert(sizeof(MarchingCubesLookup) == item.type->size);
				for (auto i = 0; i < 256; i++)
				{
					memcpy(p, &MarchingCubesLookup[i], sizeof(MarchingCubesLookupItem));
					p += sizeof(MarchingCubesLookupItem);
				}
				buf_marching_cubes_loopup.upload(cb.get());
			}
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, dsl));
			ds->set_buffer("MarchingCubesLookup"_h, 0, buf_marching_cubes_loopup.buf.get());
			ds->set_image("volume_data"_h, 0, data_map->get_view(), graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge, graphics::BorderColorBlack));
			ds->update();
			prm.set_ds(""_h, ds.get());

			cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(height_map->extent.xy())));
			cb->begin_renderpass(nullptr, height_map->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(1.f, 0.f, 0.f, 0.f) });

			cb->bind_pipeline(pl);
			prm.bind_dss(cb.get());
			prm.pc.mark_dirty_c("transform"_h).as<mat4>() = mat4(1.f);
			auto proj = orthoRH(-extent.x * 0.5f, +extent.x * 0.5f, -extent.z * 0.5f, +extent.z * 0.5f, 0.f, extent.y);
			proj[1][1] *= -1.f;
			auto view = lookAt(extent * vec3(0.5f, 1.f, 0.5f), extent * vec3(0.5f, 0.f, 0.5f), vec3(0.f, 0.f, -1.f));
			prm.pc.mark_dirty_c("proj_view"_h).as<mat4>() = proj * view;
			prm.pc.mark_dirty_c("extent"_h).as<vec3>() = extent;
			prm.pc.mark_dirty_c("blocks"_h).as<uvec3>() = blocks;
			for (auto z = 0; z < blocks.z; z++)
			{
				for (auto y = 0; y < blocks.y; y++)
				{
					for (auto x = 0; x < blocks.x; x++)
					{
						prm.pc.mark_dirty_c("offset"_h).as<vec3>() = vec3(x, y, z);
						prm.push_constant(cb.get());
						// 128 / 4 = 32
						cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
					}
				}
			}

			cb->end_renderpass();
			cb->image_barrier(height_map, {}, graphics::ImageLayoutShaderReadOnly);

			cb.excute();
		}
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
					if ((draw_data.categories & CateMarchingCubes))
						draw_data.volumes.emplace_back(instance_id, blocks, material_res_id);
				}
				break;
			case PassOcculder:
				if (marching_cubes)
				{
					if ((draw_data.categories & CateMarchingCubes) && cast_shadow)
						draw_data.volumes.emplace_back(instance_id, blocks, material_res_id);
				}
				break;
			case PassPickUp:
				if (marching_cubes)
				{
					if ((draw_data.categories & CateMarchingCubes))
						draw_data.volumes.emplace_back(instance_id, blocks, material_res_id);
				}
				break;
			case PassTransformFeedback:
				if (marching_cubes)
				{
					if ((draw_data.categories & CateMarchingCubes))
						draw_data.volumes.emplace_back(instance_id, blocks, material_res_id);
				}
				break;
			}
		}, "volume"_h);
		node->measurers.add([this](AABB& b) {
			if (data_map)
			{
				auto pos = node->global_pos();
				b.expand(AABB(pos, pos + extent * node->global_scl()));
			}
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
