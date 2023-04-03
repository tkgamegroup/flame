#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cImage : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		bool auto_size = true;
		// Reflect
		virtual void set_auto_size(bool v) = 0;

		// Reflect
		std::filesystem::path image_name;
		// Reflect
		virtual void set_image_name(const std::filesystem::path& image_name) = 0;

		graphics::ImagePtr image = nullptr;

		struct Create
		{
			virtual cImagePtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
