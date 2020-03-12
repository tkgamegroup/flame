#pragma once

#include <flame/universe/component.h>

namespace flame
{
	namespace graphics
	{
		struct FontAtlas;
	}

	struct cElement;
	struct cText;
	struct cEventReceiver;
	struct cStyleColor2;

	struct cCombobox : Component
	{
		cText* text;
		cEventReceiver* event_receiver;

		int idx;

		cCombobox() :
			Component("cCombobox")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_index(int idx, void* sender = nullptr);

		FLAME_UNIVERSE_EXPORTS static cCombobox* create();
	};

	struct cComboboxItem : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor2* style;

		int idx;

		cComboboxItem() :
			Component("cComboboxItem")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cComboboxItem* create();
	};
}
