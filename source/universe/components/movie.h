#pragma once

#include "../component.h"

namespace flame
{
	// Reflect ctor
	struct cMovie : Component
	{
		// Reflect requires
		cElementPtr element = nullptr;

		// Reflect
		cvec4 tint_col = cvec4(255);
		// Reflect
		virtual void set_tint_col(const cvec4& col) = 0;

		// Reflect
		float speed = 1.f;

		std::vector<graphics::ImageDesc> images;
		int play_index = 0;

		struct Create
		{
			virtual cMoviePtr operator()(EntityPtr) = 0;
		};
		// Reflect static
		FLAME_UNIVERSE_API static Create& create;
	};
}
