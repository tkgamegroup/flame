#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cStretchedImage : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		std::filesystem::path image_name;
		// Reflect
		virtual void set_image_name(const std::filesystem::path& image_name) = 0;

		// Reflect
		cvec4 tint_col = cvec4(255);
		// Reflect
		virtual void set_tint_col(const cvec4& col) = 0;

		// Reflect
		vec4 border = vec4(1.f);
		// Reflect
		virtual void set_border(const vec4& border) = 0;

		graphics::ImagePtr image = nullptr;

		struct Create
		{
			virtual cStretchedImagePtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
