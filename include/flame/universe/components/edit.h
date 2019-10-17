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

	struct cEdit : Component
	{
		cElement* element;
		cText* text;
		cEventReceiver* event_receiver;

		uint cursor;

		cEdit() :
			Component("Edit")
		{
		}

		Listeners<void(void* c, const wchar_t* text)> changed_listeners;

		FLAME_UNIVERSE_EXPORTS void trigger_changed();

		FLAME_UNIVERSE_EXPORTS static cEdit* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_edit(float width, graphics::FontAtlas* font_atlas, float sdf_scale);
}
