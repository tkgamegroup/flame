#pragma once

#include <flame/universe/components/aligner.h>

namespace flame
{
	struct cElementPrivate;

	struct cAlignerPrivate : cAligner  // R ~ on_*
	{
		Align alignx = AlignMin;
		Align aligny = AlignMin;

		float width_factor = 1.f;
		float height_factor = 1.f;

		bool absolute = false;

		Vec4f margin = Vec4f(0.f);

		bool only_basic = false;

		Vec2f desired_size = Vec2f(0.f);

		cElementPrivate* element = nullptr; // R ref

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

		Vec4f get_margin() const override { return margin; }
		void set_margin(const Vec4f& m) override;

		bool get_only_basic() const override { return only_basic; }
		void set_only_basic(bool o) override;

		void on_added() override;
	};
}
