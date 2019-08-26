#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct Canvas;
	}

	/*
			   pos                        size.x()
				   +------------------------------------------------
				   |	              top inner padding
				   |			****************************
				   |	 left   *                          *  right
		  size.y() |	 inner  *          content         *  inner
				   |	padding *                          * padding
				   |	        ****************************
				   |			     bottom inner padding
	*/

	struct cElement : Component
	{
		cElement* p_element;

		float x;
		float y;
		float scale;
		float width;
		float height;

		Vec4f inner_padding; // L T R B

		float alpha;

		bool draw;
		float background_round_radius;
		uint background_round_flags;
		float background_frame_thickness;
		Vec4c background_color;
		Vec4c background_frame_color;
		float background_shadow_thickness;

		bool clip_children;

		graphics::Canvas* canvas;

		float global_x;
		float global_y;
		float global_scale;
		float global_width;
		float global_height;
		Vec4f scissor;
		bool cliped;

		bool contains(const Vec2f& pos) const
		{
			return rect_contains(Vec4f(global_x, global_y, global_x + global_width, global_y + global_height), pos);
		}

		cElement() :
			Component("Element")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cElement() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_added() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cElement* create(graphics::Canvas* canvas = nullptr);
	};
}
