#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

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

		cElement* p_element;
		graphics::Canvas* canvas;
		Vec2f global_pos;
		float global_scale;
		Vec2f global_size;
		Vec4f scissor;
		bool cliped;

		float inner_padding_horizontal() const
		{
			return inner_padding[0] + inner_padding[2];
		}

		float inner_padding_vertical() const
		{
			return inner_padding[1] + inner_padding[3];
		}

		bool contains(const Vec2f& pos) const
		{
			return rect_contains(Vec4f(global_pos, global_pos + global_size), pos) && rect_contains(scissor, pos);
		}

		cElement() :
			Component("Element")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void on_entity_added_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;
		FLAME_UNIVERSE_EXPORTS virtual Component* copy() override;

		FLAME_UNIVERSE_EXPORTS static cElement* create(graphics::Canvas* canvas = nullptr);
	};
}
