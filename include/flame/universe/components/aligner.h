#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cAligner : Component
	{
		cElement* element;

		Alignx x_align;
		Aligny y_align;
		Vec2f min_size;
		SizePolicy width_policy;
		float width_factor;
		SizePolicy height_policy;
		float height_factor;
		bool using_padding; // using layout's padding

		cAligner() :
			Component("Aligner")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cAligner* create();
	};
}
