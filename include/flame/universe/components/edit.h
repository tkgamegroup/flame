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
		uint select_end; // can be smaller than start

		void set_select(uint start, int length = 0)
		{
			select_start = start;
			select_end = start + length;
		}

		cEdit() :
			Component("cEdit")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cEdit* create();
	};
}
