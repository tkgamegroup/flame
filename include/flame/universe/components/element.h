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
		float x;
		float y;
		float scale;
		float width;
		float height;

		float global_x;
		float global_y;
		float global_scale;
		float global_width;
		float global_height;

		Vec4f inner_padding; // L T R B
		float layout_padding;

		float alpha;

		bool draw;
		Vec4f background_offset; // L T R B
		float background_round_radius;
		uint background_round_flags;
		float background_frame_thickness;
		Vec4c background_color;
		Vec4c background_frame_color;
		float background_shadow_thickness;

		graphics::Canvas* canvas;

		bool contains(const Vec2f& pos) const
		{
			return rect_contains(Vec4f(global_x, global_y, global_x + global_width, global_y + global_height), pos);
		}

		FLAME_UNIVERSE_EXPORTS cElement(Entity* e);
		FLAME_UNIVERSE_EXPORTS virtual ~cElement() override;

		FLAME_UNIVERSE_EXPORTS virtual void on_add_to_parent() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cElement* create(Entity* e, graphics::Canvas* canvas = nullptr);
	};
}
