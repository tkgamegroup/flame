#pragma once

#include <flame/universe/systems/layout_management.h>
#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cAligner;

	struct sLayoutManagement;

	struct cLayout : Component
	{
		sLayoutManagement* management;

		cElement* element;
		cAligner* aligner;

		LayoutType type;
		uint column;
		float item_padding;
		bool width_fit_children;
		bool height_fit_children;
		int fence;
		Vec2f scroll_offset;

		Vec2f content_size;

		cLayout() :
			Component("cLayout")
		{
		}

		void mark_dirty()
		{
			if (management)
				management->add_to_update_list(this);
		}

		FLAME_UNIVERSE_EXPORTS void set_x_scroll_offset(float x);
		FLAME_UNIVERSE_EXPORTS void set_y_scroll_offset(float y);
		FLAME_UNIVERSE_EXPORTS void set_column(uint c);

		FLAME_UNIVERSE_EXPORTS static cLayout* create(LayoutType type);
	};
}
