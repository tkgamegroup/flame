#include "element_private.h"
#include "layout_private.h"

namespace flame
{
	void cLayoutPrivate::set_type(Type type)
	{

	}

	void cLayoutPrivate::set_spacing(float spacing)
	{
	}

	void cLayoutPrivate::update()
	{

	}

	struct cLayoutCreate : cLayout::Create
	{
		cLayoutPtr operator()(EntityPtr) override
		{
			return new cLayoutPrivate();
		}
	}cLayout_create;
	cLayout::Create& cLayout::create = cLayout_create;
}
