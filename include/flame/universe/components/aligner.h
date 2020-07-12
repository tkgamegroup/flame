#pragma once

#include <flame/universe/component.h>

namespace flame
{
//	struct cElement;
//
	struct FLAME_R(cAligner : Component, all)
	{
		inline static auto type_name = "cAligner";
		inline static auto type_hash = ch(type_name);

		cAligner() :
			Component(type_name, type_hash, true)
		{
		}

		//virtual AlignFlags get_x_align_flags() const = 0;

//		cElement* element;
//
//		FLAME_RV(Vec4f, margin); // L T R B
//		FLAME_RV(AlignFlag, x_align_flags, m);
//		FLAME_RV(AlignFlag, y_align_flags, m);
//		float min_width;
//		float min_height;
//		float width_factor;
//		float height_factor;
//
//		cAligner() :
//			Component("cAligner")
//		{
//		}
//
//		FLAME_UNIVERSE_EXPORTS void set_x_align_flags(uint a);
//		FLAME_UNIVERSE_EXPORTS void set_y_align_flags(uint a);
//		FLAME_UNIVERSE_EXPORTS void set_min_width(float w);
//		FLAME_UNIVERSE_EXPORTS void set_min_height(float h);
//		FLAME_UNIVERSE_EXPORTS void set_width_factor(float f);
//		FLAME_UNIVERSE_EXPORTS void set_height_factor(float f);

		FLAME_UNIVERSE_EXPORTS static cAligner* create();
	};
}
