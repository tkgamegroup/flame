#include "../entity_private.h"
#include "element_private.h"
#include "aligner_private.h"

namespace flame
{
	void cAlignerPrivate::set_alignx(Align a)
	{
		if (alignx == a)
			return;
		alignx = a;
		data_changed(S<"alignx"_h>);
	}

	void cAlignerPrivate::set_aligny(Align a)
	{
		if (aligny == a)
			return;
		aligny = a;
		data_changed(S<"aligny"_h>);
	}

	void cAlignerPrivate::set_width_factor(float f)
	{
		if (width_factor == f)
			return;
		width_factor = f;
		data_changed(S<"width_factor"_h>);
	}

	void cAlignerPrivate::set_height_factor(float f)
	{
		if (height_factor == f)
			return;
		height_factor = f;
		data_changed(S<"height_factor"_h>);
	}

	void cAlignerPrivate::set_absolute(bool a)
	{
		if (absolute == a)
			return;
		absolute = a;
		data_changed(S<"absolute"_h>);
	}

	void cAlignerPrivate::set_margin(const vec4& m)
	{
		if (margin == m)
			return;
		margin = m;
		margin_size[0] = m[0] + m[2];
		margin_size[1] = m[1] + m[3];
		data_changed(S<"margin"_h>);
	}

	void cAlignerPrivate::set_include_in_layout(bool o)
	{
		if (include_in_layout == o)
			return;
		include_in_layout = o;
		data_changed(S<"include_in_layout"_h>);
	}

	void cAlignerPrivate::on_added()
	{
		element->mark_size_dirty();
	}

	cAligner* cAligner::create()
	{
		return f_new<cAlignerPrivate>();
	}
}
