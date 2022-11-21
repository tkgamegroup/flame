#include "node_private.h"
#include "procedure_volume_private.h"
#include "volume_private.h"
#include "../../graphics/image.h"
#include "../../graphics/command.h"
#include "../../graphics/extension.h"

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

	void cProcedureVolumePrivate::set_size_per_block(uint size)
	{
		if (size_per_block == size)
			return;
		size_per_block = size;

		build_volume();
		volume->node->mark_transform_dirty();
		data_changed("size_per_block"_h);
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

		auto extent = volume->blocks * size_per_block;
		if (!data_map || data_map->extent != extent)
		{
			delete data_map;

			data_map = graphics::Image::create(graphics::Format_R8_UNORM, extent, graphics::ImageUsageSampled | graphics::ImageUsageStorage, graphics::ImageUsageTransferDst);
			volume->set_data_map_name(L"0x" + wstr_hex((uint64)data_map));
		}

		const auto noise_ext = 16;
		auto noise_texture = graphics::Image::create(graphics::Format_R8_UNORM, uvec3(noise_ext), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled);
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
		graphics::InstanceCommandBuffer cb;
		cb->image_barrier(noise_texture, {}, graphics::ImageLayoutTransferDst);
		cb->copy_buffer_to_image(noise_data.get(), noise_texture, graphics::BufferImageCopy(uvec3(noise_ext)));
		cb->image_barrier(noise_texture, {}, graphics::ImageLayoutShaderReadOnly);

		auto pl = graphics::ComputePipeline::get(L"flame\\shaders\\volume\\procedure.pipeline", {});

		graphics::PipelineResourceManager prm;
		prm.init(pl->layout, graphics::PipelineCompute);
		std::unique_ptr<graphics::DescriptorSet> ds(graphics::DescriptorSet::create(nullptr, prm.get_dsl(""_h)));
		ds->set_image("noise"_h, 0, noise_texture->get_view(), graphics::Sampler::get(graphics::FilterLinear, graphics::FilterLinear, false, graphics::AddressRepeat));
		ds->set_image("dst"_h, 0, data_map->get_view(), nullptr);
		ds->update();
		prm.set_ds(""_h, ds.get());

		cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderStorage);
		cb->bind_pipeline(pl);
		prm.bind_dss(cb.get());
		prm.pc.item("extent"_h).set(volume->extent);
		prm.pc.item("cells"_h).set(extent);
		prm.pc.item("offset"_h).set(offset);
		prm.pc.item("plane0"_h).set(plane0);
		prm.pc.item("plane1"_h).set(plane1);
		prm.pc.item("amplitude_scale"_h).set(amplitude_scale);
		prm.pc.item("structure_octaves"_h).set((uint)structure_octaves.size());
		prm.pc.item("detail_octaves"_h).set((uint)detail_octaves.size());
		prm.pc.item("structure_amplitudes"_h).set(structure_octaves.data(), sizeof(float) * structure_octaves.size());
		prm.pc.item("detail_amplitudes"_h).set(detail_octaves.data(), sizeof(float) * detail_octaves.size());
		prm.push_constant(cb.get());
		cb->dispatch(extent / 4U);
		cb->image_barrier(data_map, {}, graphics::ImageLayoutShaderReadOnly);

		cb.excute();
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
