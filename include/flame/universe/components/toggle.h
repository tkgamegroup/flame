#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cEventReceiver;
	struct cStyleColor2;

	struct cToggle : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor2* style;

		bool toggled;

		cToggle() :
			Component("cToggle")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_toggled(bool toggled, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cToggle* create();
	};
}
