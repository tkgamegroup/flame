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
		SizePolicy$ height_policy;

		cAligner() :
			Component("Aligner")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cAligner() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_added() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cAligner* create();
	};
}
