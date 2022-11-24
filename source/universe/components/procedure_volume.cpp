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

	void cProcedureVolumePrivate::set_plane0(const vec4& plane)
	{
		if (plane0 == plane)
			return;
		plane0 = plane;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("plane0"_h);
	}

	void cProcedureVolumePrivate::set_plane1(const vec4& plane)
	{
		if (plane1 == plane)
			return;
		plane1 = plane;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("plane1"_h);
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
		if (structure_octaves == octaves)
			return;
		structure_octaves = octaves;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("structure_octaves"_h);
	}

	void cProcedureVolumePrivate::set_detail_octaves(const std::vector<float>& octaves)
	{
		if (detail_octaves == octaves)
			return;
		detail_octaves = octaves;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("detail_octaves"_h);
	}

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
			prm.pc.item_d("plane0"_h).set(plane0);
			prm.pc.item_d("plane1"_h).set(plane1);
			prm.pc.item_d("amplitude_scale"_h).set(amplitude_scale);
			prm.pc.item_d("structure_octaves"_h).set((uint)structure_octaves.size());
			prm.pc.item_d("detail_octaves"_h).set((uint)detail_octaves.size());
			prm.pc.itemv_d("structure_amplitudes"_h, structure_octaves.size()).set(structure_octaves.data());
			prm.pc.itemv_d("detail_amplitudes"_h, detail_octaves.size()).set(detail_octaves.data());
			prm.push_constant(cb.get());
			cb->dispatch(image_size / 4U);
			cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderReadOnly);

			cb.excute();
		}

		auto img_dep = std::unique_ptr<graphics::Image>(graphics::Image::create(graphics::Format_Depth16, uvec3(512, 512, 1), graphics::ImageUsageAttachment | graphics::ImageUsageSampled | graphics::ImageUsageTransferSrc));
		{
			graphics::InstanceCommandBuffer cb;

			cb->set_viewport_and_scissor(Rect(vec2(0.f), vec2(img_dep->extent.xy())));
			cb->begin_renderpass(nullptr, img_dep->get_shader_write_dst(0, 0, graphics::AttachmentLoadClear), { vec4(1.f, 0.f, 0.f, 0.f) });

			cb->bind_vertex_buffer((graphics::BufferPtr)sRenderer::instance()->get_object("buf_vtx"_h), 0);
			cb->bind_index_buffer((graphics::BufferPtr)sRenderer::instance()->get_object("buf_idx"_h), graphics::IndiceTypeUint);

			auto rp_dep = graphics::Renderpass::get(L"flame\\shaders\\depth.rp",
				{ "dep_fmt=" + TypeInfo::serialize_t(img_dep->format) });
			//auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\volume\\marching_cubes.pipeline", { "all_shader:OCCLUDER_PASS", "mesh:PUSH_TRANSFORM", "rp=" + str(rp_dep), "cull_mode=" + TypeInfo::serialize_t(graphics::CullModeNone) });
			auto pl = graphics::GraphicsPipeline::get(L"flame\\shaders\\mesh\\mesh.pipeline", { "all_shader:OCCLUDER_PASS", "vert:PUSH_TRANSFORM", "rp=" + str(rp_dep), "cull_mode=" + TypeInfo::serialize_t(graphics::CullModeNone) });
			cb->bind_pipeline(pl);
			auto& prm = *(graphics::PipelineResourceManager*)sRenderer::instance()->get_object("prm_fwd"_h);
			prm.bind_dss(cb.get());
			auto proj = orthoRH(-volume->extent.x * 0.5f, +volume->extent.x * 0.5f, -volume->extent.z * 0.5f, +volume->extent.z * 0.5f, 0.f, volume->extent.y);
			proj[1][1] *= -1.f;
			auto view = lookAt(volume->extent * vec3(0.5f, 1.f, 0.5f), volume->extent * vec3(0.5f, 0.f, 0.5f), vec3(0.f, 0.f, -1.f));
			prm.pc.item_d("transform"_h).set(proj * view);
			prm.pc.item_d("index"_h).set(ivec4((volume->instance_id << 16) + volume->material_res_id, 0, 0, 0));
			prm.push_constant(cb.get());
			auto& mr = sRenderer::instance()->get_mesh_res_info(0);
			cb->draw_indexed(mr.idx_cnt, mr.idx_off, mr.vtx_off, 1, 0);
			//for (auto z = 0; z < volume->blocks.z; z++)
			//{
			//	for (auto y = 0; y < volume->blocks.y; y++)
			//	{
			//		for (auto x = 0; x < volume->blocks.x; x++)
			//		{
			//			prm.pc.item_d("offset"_h).set(vec3(x, y, z));
			//			prm.push_constant(cb.get());
			//			// 128 / 4 = 32
			//			cb->draw_mesh_tasks(uvec3(32 * 32 * 32, 1, 1));
			//		}
			//	}
			//}
			cb->end_renderpass();
			cb->image_barrier(img_dep.get(), {}, graphics::ImageLayoutShaderReadOnly);

			graphics::Debug::start_capture_frame();
			cb.excute();
			graphics::Debug::end_capture_frame();
		}

		{
			graphics::InstanceCommandBuffer cb;
			cb->image_barrier(noise_texture, {}, graphics::ImageLayoutTransferDst);
			cb->copy_buffer_to_image(noise_data.get(), noise_texture, graphics::BufferImageCopy(uvec3(noise_ext)));
			cb->image_barrier(noise_texture, {}, graphics::ImageLayoutShaderReadOnly);

			auto pl = graphics::ComputePipeline::get(L"flame\\shaders\\volume\\build_path.pipeline", {});
			auto prm = graphics::PipelineResourceManager(pl->layout, graphics::PipelineCompute);
			auto dsl = prm.get_dsl(""_h);
			auto buf_path = graphics::StorageBuffer(graphics::BufferUsageUniform, dsl->get_buf_ui("Path"_h));
			buf_path.item_d("n"_h).set(2U);
			auto pp = buf_path.itemv_d("p"_h, 2);
			pp.at(0).set(vec2(64, 0));
			pp.at(1).set(vec2(64, 128));
			buf_path.upload(cb.get());
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			ds->set_buffer("Path"_h, 0, buf_path.buf.get());
			ds->set_image("dep"_h, 0, img_dep->get_view(), nullptr);
			ds->set_image("dst"_h, 0, data_map->get_view(), nullptr);
			ds->update();
			prm.set_ds(""_h, ds.get());

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
