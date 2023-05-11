#include "element_private.h"
#include "layout_private.h"

namespace flame
{
	void cLayoutPrivate::set_type(ElementLayoutType type)
	{

	}

	void cLayoutPrivate::set_padding(const vec4& padding)
	{

	}

	void cLayoutPrivate::set_item_spacing(float spacing)
	{

	}

	void cLayoutPrivate::set_auto_width(bool auto_width)
	{

	}

	void cLayoutPrivate::set_auto_height(bool auto_height)
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
