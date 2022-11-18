#pragma once

#include "procedure_volume.h"

namespace flame
{
	struct cProcedureVolumePrivate : cProcedureVolume
	{
		cProcedureVolumePrivate();
		~cProcedureVolumePrivate();

		graphics::ImagePtr data_map = nullptr;

		void set_size_per_block(uint size) override;
		void set_offset(float off) override;
		void set_amplitude_scale(float scale) override;
		void set_seed(uint seed) override;
		void set_structure_octaves(const std::vector<float>& octaves) override;
		void set_detail_octaves(const std::vector<float>& octaves) override;

		void build_volume();

		void on_init() override;
	};
}
