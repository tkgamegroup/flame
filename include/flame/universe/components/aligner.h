#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;

	struct cAligner : Component
	{
		cElement* element;

		Alignx x_align_;
		Aligny y_align_;
		SizePolicy width_policy_;
		float min_width_;
		float width_factor_;
		SizePolicy height_policy_;
		float min_height_;
		float height_factor_;
		bool using_padding_; // using layout's padding

		cAligner() :
			Component("Aligner")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_x_align(Alignx a, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_y_align(Aligny a, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_width_policy(SizePolicy p, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_min_width(float w, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_width_factor(float f, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height_policy(SizePolicy p, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_min_height(float h, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height_factor(float f, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_using_padding(bool v, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cAligner* create();
	};
}
