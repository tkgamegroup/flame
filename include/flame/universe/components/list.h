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
	struct cStyleColor;
	struct cStyleTextColor;
	struct cList;

	struct cListItem : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor* background_style;
		cStyleTextColor* text_style;
		cList* list;

		Vec4c unselected_color_normal;
		Vec4c unselected_color_hovering;
		Vec4c unselected_color_active;
		Vec4c unselected_text_color_normal;
		Vec4c unselected_text_color_else;
		Vec4c selected_color_normal;
		Vec4c selected_color_hovering;
		Vec4c selected_color_active;
		Vec4c selected_text_color_normal;
		Vec4c selected_text_color_else;

		cListItem() :
			Component("ListItem")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cListItem* create();
	};

	struct cList : Component
	{
		cEventReceiver* event_receiver;

		Entity* selected;

		cList() :
			Component("List")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_selected(Entity* e, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS static cList* create(bool select_air_when_clicked = true);
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_list(bool size_fit_parent);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_listitem(graphics::FontAtlas* font_atlas, float sdf_scale, const std::wstring& text);
}
