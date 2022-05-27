#pragma once

#include "../component.h"

namespace flame
{
	struct cEnvironment : Component
	{
		/// Reflect
		std::filesystem::path sky_map_name; // will also load belonging _rad, _irr and _brdf map
		/// Reflect
		virtual void set_sky_map_name(const std::filesystem::path& name) = 0;

		struct Create
		{
			virtual cEnvironmentPtr operator()() = 0;
		};
		FLAME_UNIVERSE_API static Create& create;
	};
}
