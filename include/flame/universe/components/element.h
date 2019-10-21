#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement : Component
	{
		Vec2f pos;
		float scale;
		Vec2f size;
		Vec4f inner_padding; // L T R B
		float alpha;
		Vec4f roundness;
		float frame_thickness;
		Vec4c color;
		Vec4c frame_color;
		float shadow_thickness;
		bool clip_children;

		Vec2f global_pos;
		float global_scale;
		Vec2f global_size;
		bool cliped;
		Vec4f cliped_rect;

		float inner_padding_horizontal() const
		{
			return inner_padding[0] + inner_padding[2];
		}

		float inner_padding_vertical() const
		{
			return inner_padding[1] + inner_padding[3];
		}

		cElement() :
			Component("Element")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}
