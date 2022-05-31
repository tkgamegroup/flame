#pragma once

#include "../component.h"

namespace flame
{
	/// Reflect ctor
	struct cEnvironment : Component
	{
		/// Reflect
		std::filesystem::path sky_map_name; // will also load belonging _irr and _rad map
		/// Reflect
		virtual void set_sky_map_name(const std::filesystem::path& name) = 0;

		graphics::ImagePtr sky_map = nullptr;
		graphics::ImagePtr sky_irr_map = nullptr;
		graphics::ImagePtr sky_rad_map = nullptr;

		struct Create
		{
			virtual cEnvironmentPtr operator()(EntityPtr e) = 0;
		};
		/// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
