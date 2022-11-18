#pragma once

#include "volume.h"

namespace flame
{
	struct cVolumePrivate : cVolume
	{
		bool dirty = true;

		void set_extent(const vec3& extent) override;
		void set_blocks(const uvec3& blocks) override;
		void set_data_map_name(const std::filesystem::path& name) override;
		void set_material_name(const std::filesystem::path& name) override;
		void set_cast_shadow(bool v) override;

		~cVolumePrivate();
		void on_init() override;

		void on_active() override;
		void on_inactive() override;
	};
}
