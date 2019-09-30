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
			Component("ComboboxItem")
		{
		}

		FLAME_UNIVERSE_EXPORTS virtual void start() override;

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

		FLAME_UNIVERSE_EXPORTS void* add_changed_listener(void (*listener)(void* c, int idx), const Mail<>& capture);

		FLAME_UNIVERSE_EXPORTS void remove_changed_listener(void* ret_by_add);

		FLAME_UNIVERSE_EXPORTS void set_index(int idx, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS virtual void start() override;
		FLAME_UNIVERSE_EXPORTS virtual void update() override;

		FLAME_UNIVERSE_EXPORTS static cCombobox* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_combobox(float width, graphics::FontAtlas* font_atlas, float sdf_scale, Entity* root, const std::vector<std::wstring>& items);
}
