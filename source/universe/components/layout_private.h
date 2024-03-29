#pragma once

#include "layout.h"

namespace flame
{
	struct cLayoutPrivate : cLayout
	{
		int updated_frame = -1;

		void set_type(ElementLayoutType type) override;
		void set_padding(const vec4& padding) override;
		void set_item_spacing(float spacing) override;
		void set_columns(uint columns) override;
		void set_auto_width(bool auto_width) override;
		void set_auto_height(bool auto_height) override;

		void on_active() override;

		void update_layout();
	};
}
