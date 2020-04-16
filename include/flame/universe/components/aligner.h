#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cAligner : Component
	{
		cElement* element;

		AlignFlag x_align_flags;
		AlignFlag y_align_flags;
		float min_width;
		float min_height;
		float width_factor;
		float height_factor;

		cAligner() :
			Component("cAligner")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_x_align_flags(uint a, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_y_align_flags(uint a, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_min_width(float w, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_min_height(float h, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_width_factor(float f, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height_factor(float f, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cAligner* create();
	};
}
