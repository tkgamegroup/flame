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

		delete data_map;
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

	void cProcedureVolumePrivate::set_paths_count(uint count)
	{
		if (paths_count == count)
			return;
		paths_count = count;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("paths_count"_h);
	}

	void cProcedureVolumePrivate::build_volume()
	{
		graphics::Queue::get()->wait_idle();

		auto image_size = volume->blocks * image_size_per_block;

		delete data_map;
		delete splash_map;

		data_map = graphics::Image::create(graphics::Format_R8_UNORM, image_size, graphics::ImageUsageSampled | graphics::ImageUsageStorage | graphics::ImageUsageTransferDst);
		splash_map = graphics::Image::create(graphics::Format_R8G8B8A8_UNORM, image_size, graphics::ImageUsageSampled | graphics::ImageUsageStorage | graphics::ImageUsageTransferDst);

		const auto noise_ext = 16;
		std::unique_ptr<graphics::Image> noise_textures[3];
		for (auto i = 0; i < countof(noise_textures); i++)
			noise_textures[i].reset(graphics::Image::create(graphics::Format_R16G16B16A16_UNORM, uvec3(noise_ext), graphics::ImageUsageSampled | graphics::ImageUsageTransferDst));
		srand(seed ? seed : time(0));
		auto noise_texture_size = noise_textures[0]->data_size;
		graphics::StagingBuffer noise_data(noise_texture_size * countof(noise_textures));
		for (auto i = 0; i < countof(noise_textures); i++)
		{
			auto noise_pdata = (char*)noise_data->mapped + i * noise_texture_size;
			for (auto z = 0; z < noise_ext; z++)
			{
				auto zoff = z * noise_ext * noise_ext;
				for (auto y = 0; y < noise_ext; y++)
				{
					auto yoff = y * noise_ext;
					for (auto x = 0; x < noise_ext; x++)
					{
						auto data = (uint*)(noise_pdata + (zoff + yoff + x) * sizeof(ushort) * 4);
						data[0] = packUnorm2x16(vec2(linearRand(0.f, 1.f), linearRand(0.f, 1.f)));
						data[1] = packUnorm2x16(vec2(linearRand(0.f, 1.f), linearRand(0.f, 1.f)));
						//noise_pdata[zoff + yoff + x] = linearRand(0.f, 1.f);
					}
				}
			}
		}

		{
			graphics::InstanceCommandBuffer cb;

			for (auto i = 0; i < countof(noise_textures); i++)
			{
				cb->image_barrier(noise_textures[i].get(), {}, graphics::ImageLayoutTransferDst);
				cb->copy_buffer_to_image(noise_data.get(), noise_textures[i].get(), graphics::BufferImageCopy(uvec3(noise_ext), i * noise_texture_size));
				cb->image_barrier(noise_textures[i].get(), {}, graphics::ImageLayoutShaderReadOnly);
			}

			auto pl = graphics::ComputePipeline::get(L"flame\\shaders\\volume\\procedure.pipeline", {});
			auto prm = graphics::PipelineResourceManager(pl->layout, graphics::PipelineCompute);
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			for (auto i = 0; i < countof(noise_textures); i++)
			{
				ds->set_image("noise_textures"_h, i, noise_textures[i]->get_view(), graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat));
			}
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

			graphics::Debug::start_capture_frame();
			cb.excute();
			graphics::Debug::end_capture_frame();
		}

		auto img_dep = volume->height_map;
		for (auto t = 0; t < paths_count; t++)
		{
			auto extent = volume->extent;
			Curve curve;
			for (auto i = 0; i < 8; i++)
			{
				curve.ctrl_points.push_back(vec3(linearRand(0.f, extent.x), 0.f, linearRand(0.f, extent.z)));
			}
			curve.segment_length = 4.f;
			curve.update();
			
			std::vector<float> heights(curve.vertices.size());
			for (auto i = 0; i < heights.size(); i++)
			{
				heights[i] = (1.f - img_dep->linear_sample(curve.vertices[i].xz / extent.xz).r) * extent.y;
			}
			float avg_height = 0.f;
			for (auto i = 0; i < heights.size(); i++)
			{
				avg_height += heights[i];
			}
			avg_height /= heights.size();
			float max_slope = 1.f;
			for (auto i = 1; i < heights.size() - 1; i++)
			{
				auto diff_h = abs(heights[i + 1] - heights[i - 1]);
				auto diff_xz = distance(curve.vertices[i - 1], curve.vertices[i + 1]);
				max_slope = max(max_slope, diff_h / diff_xz);
			}
			for (auto i = 0; i < heights.size(); i++)
			{
				curve.vertices[i].y = avg_height + (heights[i] - avg_height) / max_slope;
			}

			graphics::InstanceCommandBuffer cb;

			auto pl = graphics::ComputePipeline::get(L"flame\\shaders\\volume\\build_path.pipeline", {});
			auto prm = graphics::PipelineResourceManager(pl->layout, graphics::PipelineCompute);
			auto dsl = prm.get_dsl(""_h);
			auto buf_path = graphics::StorageBuffer(graphics::BufferUsageUniform, dsl->get_buf_ui("Path"_h));
			buf_path.item_d("n"_h).set((uint)curve.vertices.size());
			auto pp = buf_path.itemv_d("p"_h, curve.vertices.size());
			for (auto i = 0; i < curve.vertices.size(); i++)
			{
				pp.at(i).set(curve.vertices[i]);
			}
			buf_path.upload(cb.get());
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			ds->set_buffer("Path"_h, 0, buf_path.buf.get());
			ds->set_image("dep"_h, 0, img_dep->get_view(), nullptr);
			ds->set_image("data_map"_h, 0, data_map->get_view(), nullptr);
			ds->set_image("splash_map"_h, 0, splash_map->get_view(), nullptr);
			ds->update();
			prm.set_ds(""_h, ds.get());

			cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderStorage);
			cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderStorage);
			cb->bind_pipeline(pl);
			prm.bind_dss(cb.get());
			prm.pc.item_d("extent"_h).set(volume->extent);
			prm.pc.item_d("cells"_h).set(image_size);
			prm.pc.item_d("avg_height"_h).set(avg_height);
			prm.pc.item_d("max_slope"_h).set(max_slope);
			prm.push_constant(cb.get());
			cb->dispatch(image_size / 4U);
			cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderReadOnly);
			cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderReadOnly);

			cb.excute();
		}

		{
			graphics::InstanceCommandBuffer cb;

			auto pl = graphics::ComputePipeline::get(L"flame\\shaders\\volume\\auto_splash.pipeline", {});
			auto prm = graphics::PipelineResourceManager(pl->layout, graphics::PipelineCompute);
			auto dsl = prm.get_dsl(""_h);
			auto ds = std::unique_ptr<graphics::DescriptorSet>(graphics::DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
			ds->set_image("data_map"_h, 0, data_map->get_view(), nullptr);
			ds->set_image("splash_map"_h, 0, splash_map->get_view(), nullptr);
			ds->update();
			prm.set_ds(""_h, ds.get());

			cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderStorage);
			cb->bind_pipeline(pl);
			prm.bind_dss(cb.get());
			prm.pc.item_d("extent"_h).set(volume->extent);
			prm.pc.item_d("cells"_h).set(image_size);
			prm.push_constant(cb.get());
			cb->dispatch(image_size / 4U);
			cb->image_barrier(splash_map, {}, graphics::ImageLayoutShaderReadOnly);

			cb.excute();
		}

		volume->set_data_map_name(L"0x" + wstr_hex((uint64)data_map));
		volume->set_splash_map_name(L"0x" + wstr_hex((uint64)splash_map));
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
