#pragma once

#include "layout.h"

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		void set_type(ElementLayoutType type) override;
		void set_padding(const vec4& padding) override;
		void set_item_spacing(float spacing) override;
		void set_auto_width(bool auto_width) override;
		void set_auto_height(bool auto_height) override;
	};
}
