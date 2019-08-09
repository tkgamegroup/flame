#pragma once

#include <flame/foundation/foundation.h>
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
		AttributeV<float> x;
		AttributeV<float> y;
		AttributeV<float> scale;
		AttributeV<float> width;
		AttributeV<float> height;

		AttributeV<float> global_x;
		AttributeV<float> global_y;
		AttributeV<float> global_scale;
		AttributeV<float> global_width;
		AttributeV<float> global_height;

		Vec4f inner_padding; // L T R B
		float layout_padding;

		float alpha;

		Vec4f background_offset; // L T R B
		float background_round_radius;
		uint background_round_flags;
		float background_frame_thickness;
		Vec4c background_color;
		Vec4c background_frame_color;
		float background_shadow_thickness;

		FLAME_UNIVERSE_EXPORTS virtual ~cElement() override;

		FLAME_UNIVERSE_EXPORTS virtual const char* type_name() const override;
		FLAME_UNIVERSE_EXPORTS virtual uint type_hash() const override;

		FLAME_UNIVERSE_EXPORTS virtual void on_attach() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS graphics::Canvas* canvas() const;

		FLAME_UNIVERSE_EXPORTS static cElement* create();
	};
}
