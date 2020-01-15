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
	struct cMenuButton;
	struct cCombobox;

	struct cComboboxItem : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor2* style;
		cCombobox* combobox;

		uint idx;

		cComboboxItem() :
			Component("cComboboxItem")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cComboboxItem* create();
	};

	struct cCombobox : Component
	{
		cText* text;
		cMenuButton* menu_button;

		int idx;

		cCombobox() :
			Component("cCombobox")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_index(int idx, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS static cCombobox* create();
	};
}
