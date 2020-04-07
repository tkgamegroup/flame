#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cTimer;
	struct cElement;
	struct cText;
	struct cEventReceiver;

	struct cEdit : Component
	{
		cTimer* timer;
		cElement* element;
		cText* text;
		cEventReceiver* event_receiver;

		uint select_start;
		uint select_end; // end can be smaller than start

		cEdit() :
			Component("cEdit")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cEdit* create();
	};
}
