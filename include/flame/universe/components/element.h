#pragma once

#include <flame/math.h>
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

	struct cElement$ : Component
	{
		ATTRIBUTE_NUMBER<float> x;
		ATTRIBUTE_NUMBER<float> y;
		ATTRIBUTE_NUMBER<float> scale;
		ATTRIBUTE_NUMBER<float> width;
		ATTRIBUTE_NUMBER<float> height;

		ATTRIBUTE_NUMBER<float> global_x;
		ATTRIBUTE_NUMBER<float> global_y;
		ATTRIBUTE_NUMBER<float> global_scale;
		ATTRIBUTE_NUMBER<float> global_width;
		ATTRIBUTE_NUMBER<float> global_height;

		Vec4f inner_padding; // L T R B
		float layout_padding;

		float alpha;

		Vec4f background_offset; // L T R B
		float background_round_radius;
		int background_round_flags;
		float background_frame_thickness;
		Vec4c background_color;
		Vec4c background_frame_color;
		float background_shadow_thickness;

		FLAME_UNIVERSE_EXPORTS virtual ~cElement$() override;

		FLAME_UNIVERSE_EXPORTS virtual const char* type_name() const override;
		FLAME_UNIVERSE_EXPORTS virtual uint type_hash() const override;

		FLAME_UNIVERSE_EXPORTS virtual void on_attach() override;

		FLAME_UNIVERSE_EXPORTS virtual void update(float delta_time) override;

		FLAME_UNIVERSE_EXPORTS graphics::Canvas* canvas() const;

		FLAME_UNIVERSE_EXPORTS static cElement$* create$(void* data);
	};
}
