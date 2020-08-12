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

	void cAlignerPrivate::set_include_in_layout(bool o)
	{
		if (include_in_layout == o)
			return;
		include_in_layout = o;
		Entity::report_data_changed(this, S<ch("include_in_layout")>::v);
	}

	void cAlignerPrivate::on_added()
	{
		entity->on_message(MessageElementSizeDirty);
	}

	cAligner* cAligner::create()
	{
		return f_new<cAlignerPrivate>();
	}
}
