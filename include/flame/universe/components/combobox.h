#pragma once

#include <flame/universe/component.h>

namespace flame
{
	struct cElement;
	struct cText;
	struct cEventReceiver;
	struct cStyleBackgroundColor;
	struct cMenuButton;
	struct cCombobox;

	struct cComboboxItem : Component
	{
		cText* text;
		cEventReceiver* event_receiver;
		cStyleBackgroundColor* style;
		cCombobox* combobox;

		Vec4c unselected_color_normal;
		Vec4c unselected_color_hovering;
		Vec4c unselected_color_active;
		Vec4c selected_color_normal;
		Vec4c selected_color_hovering;
		Vec4c selected_color_active;

		cComboboxItem() :
			Component("ComboboxItem")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cComboboxItem() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cComboboxItem* create();
	};

	struct cCombobox : Component
	{
		cText* text;
		cMenuButton* menu_button;

		Entity* selected;

		cCombobox() :
			Component("Combobox")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual ~cCombobox() override;

		FLAME_UNIVERSE_EXPORTS virtual void start() override;

		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cCombobox* create();
	};
}
