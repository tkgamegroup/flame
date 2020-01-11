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
	struct cStyleTextColor2;
	struct cList;

	struct cListItem : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor2* background_style;
		cStyleTextColor2* text_style;
		cList* list;

		cListItem() :
			Component("cListItem")
		{
		}

		FLAME_UNIVERSE_EXPORTS static cListItem* create();
	};

	struct cList : Component
	{
		cEventReceiver* event_receiver;

		Entity* selected;

		cList() :
			Component("cList")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_selected(Entity* e, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS static cList* create(bool select_air_when_clicked = true);
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_list(bool size_fit_parent);
	FLAME_UNIVERSE_EXPORTS Entity* create_standard_listitem(graphics::FontAtlas* font_atlas, float font_size_scale, const wchar_t* text);
}
