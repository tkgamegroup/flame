#include "element_private.h"

namespace flame
{
	void cElementPrivate::set_pos(const vec2& pos)
	{

	}

	void cElementPrivate::set_ext(const vec2& ext)
	{

	}

	void cElementPrivate::mark_transform_dirty()
	{

	}

	void cElementPrivate::mark_drawing_dirty()
	{

	}

	struct cElementCreate : cElement::Create
	{
		cElementPtr operator()(EntityPtr) override
		{
			return new cElementPrivate();
		}
	}cElement_create;
	cElement::Create& cElement::create = cElement_create;
}
