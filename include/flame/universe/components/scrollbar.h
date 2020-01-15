#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cLayout;

	struct cScrollbar : Component
	{
		cElement* element;

		cScrollbar() :
			Component("cScrollbar")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cScrollbar* create();
	};

	struct cScrollbarThumb : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cScrollbar* scrollbar;
		cElement* parent_element;
		cLayout* target_layout;

		ScrollbarType type;
		float step;

		cScrollbarThumb() :
			Component("cScrollbarThumb")
		{
		}

		FLAME_UNIVERSE_EXPORTS void update(float v);

		FLAME_UNIVERSE_EXPORTS static cScrollbarThumb* create(ScrollbarType type);
	};
}
