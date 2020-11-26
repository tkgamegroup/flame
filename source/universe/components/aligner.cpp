#include "../entity_private.h"
#include "aligner_private.h"

namespace flame
{
	void cAlignerPrivate::set_alignx(Align a)
	{
		if (alignx == a)
			return;
		alignx = a;
		Entity::report_data_changed(this, S<"alignx"_h>);
	}

	void cAlignerPrivate::set_aligny(Align a)
	{
		if (aligny == a)
			return;
		aligny = a;
		Entity::report_data_changed(this, S<"aligny"_h>);
	}

	void cAlignerPrivate::set_width_factor(float f)
	{
		if (width_factor == f)
			return;
		width_factor = f;
		Entity::report_data_changed(this, S<"width_factor"_h>);
	}

	void cAlignerPrivate::set_height_factor(float f)
	{
		if (height_factor == f)
			return;
		height_factor = f;
		Entity::report_data_changed(this, S<"height_factor"_h>);
	}

	void cAlignerPrivate::set_absolute(bool a)
	{
		if (absolute == a)
			return;
		absolute = a;
		Entity::report_data_changed(this, S<"absolute"_h>);
	}

	void cAlignerPrivate::set_margin(const vec4& m)
	{
		if (margin == m)
			return;
		margin = m;
		Entity::report_data_changed(this, S<"margin"_h>);
	}

	void cAlignerPrivate::set_include_in_layout(bool o)
	{
		if (include_in_layout == o)
			return;
		include_in_layout = o;
		Entity::report_data_changed(this, S<"include_in_layout"_h>);
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
