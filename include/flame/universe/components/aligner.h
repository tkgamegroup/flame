#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cAligner : Component
	{
		cElement* element;

		Alignx$ x_align;
		Aligny$ y_align;
		float min_width;
		float min_height;
		SizePolicy$ width_policy;
		float width_factor;
		SizePolicy$ height_policy;
		float height_factor;
		bool using_padding_in_free_layout;

		cAligner() :
			Component("Aligner")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cAligner() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cAligner* create();
	};
}
