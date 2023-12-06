#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cWorldSettings : Component
	{
		// Reflect
		std::filesystem::path filename;
		// Reflect
		virtual void set_filename(const std::filesystem::path& path) = 0;

		virtual void save() = 0;

		struct Create
		{
			virtual cWorldSettingsPtr operator()(EntityPtr e) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
