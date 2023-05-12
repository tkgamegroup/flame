#include "element_private.h"
#include "layout_private.h"

namespace flame
{
	void cLayoutPrivate::set_type(ElementLayoutType _type)
	{
		if (type == _type)
			return;
		type = _type;
		element->mark_transform_dirty();
		data_changed("type"_h);
	}

	void cLayoutPrivate::set_padding(const vec4& padding)
	{
		if (this->padding == padding)
			return;
		this->padding = padding;
		element->mark_transform_dirty();
		data_changed("padding"_h);
	}

	void cLayoutPrivate::set_item_spacing(float spacing)
	{
		if (item_spacing == spacing)
			return;
		item_spacing = spacing;
		element->mark_transform_dirty();
		data_changed("item_spacing"_h);
	}

	void cLayoutPrivate::set_auto_width(bool _auto_width)
	{
		if (auto_width == _auto_width)
			return;
		auto_width = _auto_width;
		element->mark_transform_dirty();
		data_changed("auto_width"_h);
	}

	void cLayoutPrivate::set_auto_height(bool _auto_height)
	{
		if (auto_height == _auto_height)
			return;
		auto_height = _auto_height;
		element->mark_transform_dirty();
		data_changed("auto_height"_h);
	}

	void cLayoutPrivate::on_active()
	{
		element->mark_transform_dirty();
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
