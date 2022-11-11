#pragma once

#include <flame/graphics/gui.h>
#include <flame/universe/components/volume.h>

using namespace flame;

struct ProcedureVolumeDialog : ImGui::Dialog
{
	cVolumePtr volume;

	struct Parms
	{
		int octaves = 12;
		float height = -32.f;
		float amplitude_scale = 0.05f;
		float base_strength = 0.25f;
		float amplitudes[16] = { 31.78f, 16.09f, 7.99f, 4.03f, 1.96f, 1.01f, 0.53f, 0.24f, 0.13f, 0.07f, 0.035f, 0.018f, 0.f, 0.f, 0.f, 0.f };
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
			ImGui::InputInt("Octaves", &parms.octaves);
			ImGui::InputFloat("Height", &parms.height);
			ImGui::InputFloat("Amplitude Scale", &parms.amplitude_scale);
			ImGui::InputFloat("Base Strength", &parms.base_strength);
			for (auto i = 0; i < parms.octaves; i++)
			{
				ImGui::InputFloat(std::format("Octave {} amplitude", i).c_str(), &parms.amplitudes[i]);
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
		prm.pc.item("octaves"_h).set(parms.octaves);
		prm.pc.item("height"_h).set(parms.height);
		prm.pc.item("amplitude_scale"_h).set(parms.amplitude_scale);
		prm.pc.item("base_strength"_h).set(parms.base_strength);
		prm.pc.item("amplitudes"_h).set(parms.amplitudes, sizeof(float) * countof(parms.amplitudes));
		prm.push_constant(cb.get());
		cb->dispatch(cells / 4U);
		cb->image_barrier(volume->data_map, {}, graphics::ImageLayoutShaderReadOnly);

		cb.excute();
	}
};
