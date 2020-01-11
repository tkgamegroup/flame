#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cEventReceiver;
	struct cStyleColor2;

	struct cCheckbox : Component
	{
		cElement* element;
		cEventReceiver* event_receiver;
		cStyleColor2* style;

		bool checked;

		cCheckbox() :
			Component("cCheckbox")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_checked(bool checked, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS static cCheckbox* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_checkbox();
}
