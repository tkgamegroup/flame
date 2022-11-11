#pragma once

#include <flame/graphics/gui.h>
#include <flame/universe/components/volume.h>

using namespace flame;

struct ProcedureVolumeDialog : ImGui::Dialog
{
	cVolumePtr volume;

	struct Parms
	{
		int low_octaves = 0;
		int high_octaves = 0;
		float height = 5.f;
		float amplitude_scale = 0.05f;
		float low_amplitudes[8] = { 0.f };
		float high_amplitudes[8] = { 0.f };
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
			ImGui::InputInt("Low Octaves", &parms.low_octaves); parms.low_octaves = clamp(parms.low_octaves, 0, (int)countof(parms.low_amplitudes) - 1);
			ImGui::InputInt("High Octaves", &parms.high_octaves); parms.high_octaves = clamp(parms.high_octaves, 0, (int)countof(parms.high_amplitudes) - 1);
			ImGui::InputFloat("Height", &parms.height);
			ImGui::InputFloat("Amplitude Scale", &parms.amplitude_scale);
			for (auto i = 0; i < parms.low_octaves; i++)
			{
				auto strength = 1.f / float(1 << i);
				ImGui::InputFloat(std::format("Octave {} amplitude, strength: {}", i, strength).c_str(), &parms.low_amplitudes[i]);
			}
			for (auto i = 0; i < parms.high_octaves; i++)
			{
				auto strength = float(1 << i);
				ImGui::InputFloat(std::format("Octave {} amplitude, strength: {}", i, strength).c_str(), &parms.high_amplitudes[i]);
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
		std::unique_ptr<graphics::Image> noise_texture(graphics::Image::create(graphics::Format_R8_UNORM, uvec3(noise_ext), graphics::ImageUsageTransferDst | graphics::ImageUsageSampled));
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
		cb->image_barrier(noise_texture.get(), {}, graphics::ImageLayoutTransferDst);
		cb->clear_color_image(noise_texture.get(), {}, vec4(0.f));
		cb->copy_buffer_to_image(noise_data.get(), noise_texture.get(), graphics::BufferImageCopy(uvec3(noise_ext)));
		cb->image_barrier(noise_texture.get(), {}, graphics::ImageLayoutShaderReadOnly);

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
		prm.pc.item("low_octaves"_h).set(parms.low_octaves);
		prm.pc.item("high_octaves"_h).set(parms.high_octaves);
		prm.pc.item("height"_h).set(parms.height);
		prm.pc.item("amplitude_scale"_h).set(parms.amplitude_scale);
		prm.pc.item("low_amplitudes"_h).set(parms.low_amplitudes, sizeof(float) * countof(parms.low_amplitudes));
		prm.pc.item("high_amplitudes"_h).set(parms.high_amplitudes, sizeof(float) * countof(parms.high_amplitudes));
		prm.push_constant(cb.get());
		cb->dispatch(cells / 4U);
		cb->image_barrier(volume->data_map, {}, graphics::ImageLayoutShaderReadOnly);

		cb.excute();
	}
};
