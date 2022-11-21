#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cProcedureVolume : Component
	{
		// Reflect requires
		cVolumePtr volume = nullptr;

		// Reflect
		uint size_per_block = 128;
		// Reflect
		virtual void set_size_per_block(uint size) = 0;

		// Reflect
		float offset = 5.f;
		// Reflect
		virtual void set_offset(float off) = 0;

		// Reflect
		vec4 plane0 = vec4(15.f, 0.3f, 3.f, 40.f);
		// Reflect
		virtual void set_plane0(const vec4& plane) = 0;

		// Reflect
		vec4 plane1 = vec4(5.f, 0.3f, 3.f, 40.f);
		// Reflect
		virtual void set_plane1(const vec4& plane) = 0;

		// Reflect
		float amplitude_scale = 0.05f;
		// Reflect
		virtual void set_amplitude_scale(float scale) = 0;

		// Reflect
		uint seed = 1; // 0 to use random seed
		// Reflect
		virtual void set_seed(uint seed) = 0;

		// Reflect
		std::vector<float> structure_octaves;
		// Reflect
		virtual void set_structure_octaves(const std::vector<float>& octaves) = 0;

		// Reflect
		std::vector<float> detail_octaves;
		// Reflect
		virtual void set_detail_octaves(const std::vector<float>& octaves) = 0;

		struct Create
		{
			virtual cProcedureVolumePtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
