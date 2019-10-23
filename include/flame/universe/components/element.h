#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement : Component
	{
		Vec2f pos_;
		float scale;
		Vec2f size_;
		Vec4f inner_padding_; // L T R B
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
			return inner_padding_[0] + inner_padding_[2];
		}

		float inner_padding_vertical() const
		{
			return inner_padding_[1] + inner_padding_[3];
		}

		cElement() :
			Component("Element")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_x(float x, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_y(float y, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_pos(const Vec2f& p, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_width(float w, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_height(float h, bool add = false, void* sender = nullptr);
		FLAME_UNIVERSE_EXPORTS void set_size(const Vec2f& s, bool add = false, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}
