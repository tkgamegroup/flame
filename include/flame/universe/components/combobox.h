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
	struct cStyleColor;
	struct cMenuButton;
	struct cCombobox;

	struct cComboboxItem : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor* style;
		cCombobox* combobox;

		Vec4c unselected_color_normal;
		Vec4c unselected_color_hovering;
		Vec4c unselected_color_active;
		Vec4c selected_color_normal;
		Vec4c selected_color_hovering;
		Vec4c selected_color_active;

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

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_combobox(float width, graphics::FontAtlas* font_atlas, float font_size_scale, Entity* root, const std::vector<std::wstring>& items);
}
