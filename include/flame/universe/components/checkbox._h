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

		FLAME_UNIVERSE_EXPORTS void set_checked(bool checked, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cCheckbox* create();
	};
}
