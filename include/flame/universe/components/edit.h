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

		uint cursor;

		cEdit() :
			Component("cEdit")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cEdit* create();
	};
}
