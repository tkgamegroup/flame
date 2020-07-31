#include "../entity_private.h"
#include "aligner_private.h"

namespace flame
{
	void cAlignerPrivate::set_alignx(Align a)
	{
		if (alignx == a)
			return;
		alignx = a;
		Entity::report_data_changed(this, S<ch("alignx")>::v);
	}

	void cAlignerPrivate::set_aligny(Align a)
	{
		if (aligny == a)
			return;
		aligny = a;
		Entity::report_data_changed(this, S<ch("aligny")>::v);
	}

	void cAlignerPrivate::set_width_factor(float f)
	{
		if (width_factor == f)
			return;
		width_factor = f;
		Entity::report_data_changed(this, S<ch("width_factor")>::v);
	}

	void cAlignerPrivate::set_height_factor(float f)
	{
		if (height_factor == f)
			return;
		height_factor = f;
		Entity::report_data_changed(this, S<ch("height_factor")>::v);
	}

	void cAlignerPrivate::set_absolute(bool a)
	{
		if (absolute == a)
			return;
		absolute = a;
		Entity::report_data_changed(this, S<ch("absolute")>::v);
	}

	void cAlignerPrivate::set_margin(const Vec4f& m)
	{
		if (margin == m)
			return;
		margin = m;
		Entity::report_data_changed(this, S<ch("margin")>::v);
	}

	void cAlignerPrivate::set_only_basic(bool o)
	{
		if (only_basic == o)
			return;
		only_basic = o;
		Entity::report_data_changed(this, S<ch("only_basic")>::v);
	}

	cAligner* cAligner::create()
	{
		return f_new<cAlignerPrivate>();
	}
}
