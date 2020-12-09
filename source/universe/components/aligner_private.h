#pragma once

#include "../entity_private.h"
#include <flame/universe/components/aligner.h>

namespace flame
{
	struct cElementPrivate;

	struct cAlignerPrivate : cAligner
	{
		Align alignx = AlignMin;
		Align aligny = AlignMin;

		float width_factor = 1.f;
		float height_factor = 1.f;

		bool absolute = false;

		vec4 margin = vec4(0.f);
		vec2 margin_size = vec2(0.f);

		bool include_in_layout = true;

		vec2 desired_size = vec2(0.f);

		cElementPrivate* element = nullptr;

		Align get_alignx() const override { return alignx; }
		void set_alignx(Align a) override;
		Align get_aligny() const override { return aligny; }
		void set_aligny(Align a) override;

		float get_width_factor() const override { return width_factor; }
		void set_width_factor(float f) override;
		float get_height_factor() const override { return height_factor; }
		void set_height_factor(float f) override;

		bool get_absolute() const override { return absolute; }
		void set_absolute(bool a) override;

		vec4 get_margin() const override { return margin; }
		void set_margin(const vec4& m) override;

		bool get_include_in_layout() const override { return include_in_layout; }
		void set_include_in_layout(bool o) override;

		void on_added() override;
	};
}
