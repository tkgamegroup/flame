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

	struct cToggle : Component
	{
		cEventReceiver* event_receiver;
		cStyleColor2* style;

		bool toggled;

		cToggle() :
			Component("cToggle")
		{
		}

		FLAME_UNIVERSE_EXPORTS void set_toggled(bool toggled, bool trigger_changed = true);

		FLAME_UNIVERSE_EXPORTS static cToggle* create();
	};

	FLAME_UNIVERSE_EXPORTS Entity* create_standard_toggle(graphics::FontAtlas* font_atlas, float font_size_scale, const wchar_t* text);
}
