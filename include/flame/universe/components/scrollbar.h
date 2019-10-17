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
			Component("Scrollbar")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cScrollbar* create();
	};

	struct cScrollbarThumb : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cScrollbar* scrollbar;

		ScrollbarType type;
		cLayout* target_layout;
		float step;

		float content_size;
		float v;

		cScrollbarThumb() :
			Component("ScrollbarThumb")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cScrollbarThumb* create(ScrollbarType type);
	};

	FLAME_UNIVERSE_EXPORTS Entity* wrap_standard_scrollbar(Entity* e, ScrollbarType type, bool container_fit_parent, float scrollbar_step);
}
