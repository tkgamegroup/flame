#pragma once

#include <flame/graphics/gui.h>
#include <flame/universe/components/volume.h>

using namespace flame;

struct ProcedureVolumeDialog : ImGui::Dialog
{
	cVolumePtr volume;

	struct Parms
	{
		int structure_octaves = 1;
		int detail_octaves = 0;
		float height = 5.f;
		float amplitude_scale = 0.05f;
		float structure_amplitudes[16] = { 1.01f, 0.47f, 0.247f, 0.123f, 0.063f, 0.031f, 0.017f, 0.009f };
		float detail_amplitudes[16] = { 1.98f, 4.03f, 7.97f, 8.07f };
	}parms;

	static void open(cVolumePtr volume)
	{
		auto dialog = new ProcedureVolumeDialog;
		dialog->title = "Procedure Volume";
		dialog->volume = volume;
		Dialog::open(dialog);
	}

	void draw() override
	{
		auto open = true;
		if (ImGui::Begin(title.c_str(), &open))
		{
			ImGui::InputInt("Structure Octaves", &parms.structure_octaves);
			ImGui::InputInt("Detail Octaves", &parms.detail_octaves);
			ImGui::InputFloat("Height", &parms.height);
			ImGui::InputFloat("Amplitude Scale", &parms.amplitude_scale);
			ImGui::TextUnformatted("Structure:");
			for (auto i = 0; i < parms.structure_octaves; i++)
			{
				auto strength = float(1 << i);
				ImGui::InputFloat(std::format("Octave {}(stength {}) amplitude", i, strength).c_str(), &parms.structure_amplitudes[i]);
			}
			ImGui::TextUnformatted("Detail:");
			for (auto i = 1; i <= parms.detail_octaves; i++)
			{
				auto strength = 1.f / float(1 << i);
				ImGui::InputFloat(std::format("Octave {}(stength {}) amplitude", i, strength).c_str(), &parms.detail_amplitudes[i]);
			}
			if (ImGui::Button("Generate"))
			{
				add_event([this]() {
					generate();
					return false;
				});
			}
			ImGui::SameLine();
			if (ImGui::Button("Close"))
				close();

			ImGui::End();
		}
		if (!open)
			close();
	}

	void generate()
	{
		graphics::Queue::get()->wait_idle();

		const auto noise_ext = 16;
		auto noise_texture = graphics::Image::create(graphics::Format_R8_UNORM, uvec3(noise_ext), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled);
		graphics::StagingBuffer noise_data(noise_texture->data_size);
		auto noise_pdata = (char*)noise_data->mapped;
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
		ds->set_image("dst"_h, 0, volume->data_map->get_view(), nullptr);
		ds->update();
		prm.set_ds(""_h, ds.get());

		cb->image_barrier(volume->data_map, {}, graphics::ImageLayoutShaderStorage);
		cb->bind_pipeline(pl);
		prm.bind_dss(cb.get());
		auto cells = volume->data_map->extent;
		prm.pc.item("extent"_h).set(volume->extent);
		prm.pc.item("cells"_h).set(cells);
		prm.pc.item("structure_octaves"_h).set(parms.structure_octaves);
		prm.pc.item("detail_octaves"_h).set(parms.detail_octaves);
		prm.pc.item("height"_h).set(parms.height);
		prm.pc.item("amplitude_scale"_h).set(parms.amplitude_scale);
		prm.pc.item("structure_amplitudes"_h).set(parms.structure_amplitudes, sizeof(float) * countof(parms.structure_amplitudes));
		prm.pc.item("detail_amplitudes"_h).set(parms.detail_amplitudes, sizeof(float) * countof(parms.detail_amplitudes));
		prm.push_constant(cb.get());
		cb->dispatch(cells / 4U);
		cb->image_barrier(volume->data_map, {}, graphics::ImageLayoutShaderReadOnly);

		cb.excute();
	}
};
