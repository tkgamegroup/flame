#include "node_private.h"
#include "procedure_volume_private.h"
#include "volume_private.h"
#include "../../graphics/image.h"
#include "../../graphics/command.h"
#include "../../graphics/renderpass.h"
#include "../../graphics/extension.h"
#include "../../graphics/debug.h"
#include "../systems/renderer_private.h"

namespace flame
{
	cProcedureVolumePrivate::cProcedureVolumePrivate()
	{
		structure_octaves = { 1.01f, 0.47f, 0.247f, 0.123f, 0.063f, 0.031f, 0.017f, 0.009f };
		detail_octaves = { 1.98f, 4.03f, 7.97f, 8.07f, 16.07f, 31.99f, 64.11f };
	}

	cProcedureVolumePrivate::~cProcedureVolumePrivate()
	{
		volume->data_listeners.remove("procedure_volume"_h);
	}

	void cProcedureVolumePrivate::set_image_size_per_block(uint size)
	{
		if (image_size_per_block == size)
			return;
		image_size_per_block = size;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("image_size_per_block"_h);
	}

	void cProcedureVolumePrivate::set_offset(float off)
	{
		if (offset == off)
			return;
		offset = off;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("offset"_h);
	}

	void cProcedureVolumePrivate::set_amplitude_scale(float scale)
	{
		if (amplitude_scale == scale)
			return;
		amplitude_scale = scale;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("amplitude_scale"_h);
	}
	
	void cProcedureVolumePrivate::set_seed(uint _seed)
	{
		if (seed == _seed)
			return;
		seed = _seed;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("seed"_h);
	}

	void cProcedureVolumePrivate::set_structure_octaves(const std::vector<float>& octaves)
	{
		structure_octaves = octaves;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("structure_octaves"_h);
	}

	void cProcedureVolumePrivate::set_detail_octaves(const std::vector<float>& octaves)
	{
		detail_octaves = octaves;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("detail_octaves"_h);
	}

	void cProcedureVolumePrivate::set_planes(const std::vector<vec4>& _planes)
	{
		planes = _planes;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("planes"_h);
	}

#include "../systems/marching_cubes_lookup.h"

	void cProcedureVolumePrivate::build_volume()
	{
		graphics::Queue::get()->wait_idle();

		auto image_size = volume->blocks * image_size_per_block;
		if (!data_map || data_map->extent != image_size)
		{
			delete data_map;

			data_map = graphics::Image::create(graphics::Format_R8_UNORM, image_size, graphics::ImageUsageSampled | graphics::ImageUsageStorage | graphics::ImageUsageTransferDst);
			volume->set_data_map_name(L"0x" + wstr_hex((uint64)data_map));
		}

		const auto noise_ext = 16;
		auto noise_texture = graphics::Image::create(graphics::Format_R8_UNORM, uvec3(noise_ext), graphics::ImageUsageSampled | graphics::ImageUsageTransferDst);
		graphics::StagingBuffer noise_data(noise_texture->data_size);
		auto noise_pdata = (char*)noise_data->mapped;
		srand(seed ? seed : time(0));
		for (auto z = 0; z < noise_ext; z++)
		{
			auto zoff = z * noise_ext * noise_ext;
			for (auto y = 0; y < noise_ext; y++)
			{
				auto yoff = y * noise_ext;
				for (auto x = 0; x < noise_ext; x++)
				{
					noise_pdata[zoff + yoff + x] = linearRand(0.f, 1.f) * 255.f;
				}
			}
		}

		{
			graphics::InstanceCommandBuffer cb;
			cb->image_barrier(noise_texture, {}, graphics::ImageLayoutTransferDst);
			cb->copy_buffer_to_image(noise_data.get(), noise_texture, graphics::BufferImageCopy(uvec3(noise_ext)));
			cb->image_barrier(noise_texture, {}, graphics::ImageLayoutShaderReadOnly);

			auto pl = graphics::ComputePipeline::get(L"flame\\shaders\\volume\\procedure.pipeline", {});
			auto prm = graphics::PipelineResourceManager(pl->layout, graphics::PipelineCompute);
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			ds->set_image("noise"_h, 0, noise_texture->get_view(), graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat));
			ds->set_image("dst"_h, 0, data_map->get_view(), nullptr);
			ds->update();
			prm.set_ds(""_h, ds.get());

			cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderStorage);
			cb->bind_pipeline(pl);
			prm.bind_dss(cb.get());
			prm.pc.item_d("extent"_h).set(volume->extent);
			prm.pc.item_d("cells"_h).set(image_size);
			prm.pc.item_d("offset"_h).set(offset);
			prm.pc.item_d("amplitude_scale"_h).set(amplitude_scale);
			prm.pc.item_d("structure_octaves"_h).set((uint)structure_octaves.size());
			prm.pc.item_d("detail_octaves"_h).set((uint)detail_octaves.size());
			prm.pc.item_d("planes_count"_h).set((uint)planes.size());
			prm.pc.itemv_d("structure_amplitudes"_h, structure_octaves.size()).set(structure_octaves.data());
			prm.pc.itemv_d("detail_amplitudes"_h, detail_octaves.size()).set(detail_octaves.data());
			prm.pc.itemv_d("planes"_h, planes.size()).set(planes.data());
			prm.push_constant(cb.get());
			cb->dispatch(image_size / 4U);
			cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderReadOnly);

			cb.excute();
		}

		auto img_dep = std::unique_ptr<graphics::Image>(graphics::Image::create(graphics::Format_Depth16, uvec3(512, 512, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageTransferSrc));
		{
			graphics::InstanceCommandBuffer cb;

			auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\volume\\get_height.pipeline", { "rp:dep_fmt=" + TypeInfo::serialize_t(img_dep->format),
				"all_shader:CUSTOM_INPUT",
				"all_shader:_transform=pc.transform",
				"all_shader:_proj_view=pc.proj_view",
				"all_shader:_extent=pc.extent",
				"all_shader:_blocks=pc.blocks",
				"all_shader:DATA_MAP=volume_data",
			});
			auto prm = graphics::PipelineResourceManager(pl->layout);
			auto dsl = prm.get_dsl(""_h);
			graphics::StorageBuffer buf_marching_cubes_loopup(graphics::BufferUsageStorage, dsl->get_buf_ui("MarchingCubesLookup"_h));
			{
				auto pi = buf_marching_cubes_loopup.itemv_d("items"_h, 256);
				auto pdata = pi.pdata;
				assert(sizeof(MarchingCubesLookup) == pi.size);
				for (auto i = 0; i < 256; i++)
				{
					memcpy(pdata, &MarchingCubesLookup[i], sizeof(MarchingCubesLookupItem));
					pdata += sizeof(MarchingCubesLookupItem);
				}
				buf_marching_cubes_loopup.upload(cb.get());
			}
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, dsl));
			ds->set_buffer("MarchingCubesLookup"_h, 0, buf_marching_cubes_loopup.buf.get());
			ds->set_image("volume_data"_h, 0, volume->data_map->get_view(), graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressClampToEdge, graphics::BorderColorBlack));
			ds->update();
			prm.set_ds(""_h, ds.get());

			cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(img_dep->extent.xy())));
			cb->begin_renderpass(nullptr, img_dep->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(1.f, 0.f, 0.f, 0.f) });

			cb->bind_pipeline(pl);
			prm.bind_dss(cb.get());
			prm.pc.item_d("transform"_h).set(mat4(1.f));
			auto proj = orthoRH(-volume->extent.x * 0.5f, +volume->extent.x * 0.5f, -volume->extent.z * 0.5f, +volume->extent.z * 0.5f, 0.f, volume->extent.y);
			proj[1][1] *= -1.f;
			auto view = lookAt(volume->extent * vec3(0.5f, 1.f, 0.5f), volume->extent * vec3(0.5f, 0.f, 0.5f), vec3(0.f, 0.f, -1.f));
			prm.pc.item_d("proj_view"_h).set(proj * view);
			prm.pc.item_d("extent"_h).set(volume->extent);
			prm.pc.item_d("blocks"_h).set(volume->blocks);
			for (auto z = 0; z < volume->blocks.z; z++)
			{
				for (auto y = 0; y < volume->blocks.y; y++)
				{
					for (auto x = 0; x < volume->blocks.x; x++)
					{
						prm.pc.item_d("offset"_h).set(vec3(x, y, z));
						prm.push_constant(cb.get());
						// 128 / 4 = 32
						cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
					}
				}
			}

			cb->end_renderpass();
			cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);

			cb.excute();
		}

		{
			auto extent = volume->extent;
			Curve<vec2> curve;
			for (auto i = 0; i < 2; i++)
			{
				curve.ctrl_points.push_back(vec2(linearRand(0.f, extent.x), linearRand(0.f, extent.z)));
			}
			curve.tess = 8;
			curve.update();

			graphics::InstanceCommandBuffer cb;

			auto pl = graphics::ComputePipeline::get(L"flame\\shaders\\volume\\build_path.pipeline", {});
			auto prm = graphics::PipelineResourceManager(pl->layout, graphics::PipelineCompute);
			auto dsl = prm.get_dsl(""_h);
			auto buf_path = graphics::StorageBuffer(graphics::BufferUsageUniform, dsl->get_buf_ui("Path"_h));
			buf_path.item_d("n"_h).set(2U);
			auto pp = buf_path.itemv_d("p"_h, curve.vertices.size());
			for (auto i = 0; i < curve.vertices.size(); i++)
			{
				pp.at(i).set(curve.vertices[i]);
			}
			buf_path.upload(cb.get());
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			ds->set_buffer("Path"_h, 0, buf_path.buf.get());
			ds->set_image("dep"_h, 0, img_dep->get_view(), nullptr);
			ds->set_image("dst"_h, 0, data_map->get_view(), nullptr);
			ds->update();
			prm.set_ds(""_h, ds.get());

			cb->image_barrier(noise_texture, {}, graphics::ImageLayoutTransferDst);
			cb->copy_buffer_to_image(noise_data.get(), noise_texture, graphics::BufferImageCopy(uvec3(noise_ext)));
			cb->image_barrier(noise_texture, {}, graphics::ImageLayoutShaderReadOnly);

			cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderStorage);
			cb->bind_pipeline(pl);
			prm.bind_dss(cb.get());
			prm.pc.item_d("extent"_h).set(volume->extent);
			prm.pc.item_d("cells"_h).set(image_size);
			prm.push_constant(cb.get());
			cb->dispatch(image_size / 4U);
			cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderReadOnly);

			cb.excute();
		}
	}

	void cProcedureVolumePrivate::on_init()
	{
		build_volume();

		volume->data_listeners.add([this](uint hash) {
			if (hash == "extent"_h || hash == "blocks"_h)
				build_volume();
		}, "procedure_volume"_h);
	}

	struct cProcedureVolumeCreate : cProcedureVolume::Create
	{
		cProcedureVolumePtr operator()(EntityPtr e) override
		{
			return new cProcedureVolumePrivate();
		}
	}cProcedureVolume_create;
	cProcedureVolume::Create& cProcedureVolume::create = cProcedureVolume_create;
}
